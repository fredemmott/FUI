// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// SdlSkiaVulkanWindow — see SdlSkiaVulkanWindow.hpp for scope.
//
// Deliberately simple Vulkan init: 1 graphics+present queue, BGRA8 SRGB
// swapchain, FIFO present mode (v-synced, no tearing). No MSAA; Skia does
// its own multisample inside Ganesh. The complexity sits in the swapchain
// rebuild path (resize) and the per-frame Skia surface wrapping.

#include "SdlSkiaVulkanWindow.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <skia/core/SkColorSpace.h>
#include <skia/gpu/ganesh/GrBackendSurface.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>
#include <skia/gpu/ganesh/GrBackendSemaphore.h>
#include <skia/gpu/ganesh/GrDirectContext.h>
#include <skia/gpu/ganesh/GrTypes.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>
#include <skia/gpu/ganesh/vk/GrVkBackendSemaphore.h>
#include <skia/gpu/ganesh/vk/GrVkBackendSurface.h>
#include <skia/gpu/ganesh/vk/GrVkDirectContext.h>
#include <skia/gpu/ganesh/vk/GrVkTypes.h>
#include <skia/gpu/vk/VulkanBackendContext.h>
#include <skia/gpu/vk/VulkanExtensions.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/SkiaRenderer.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/detail/renderer_detail.hpp>
#include <FredEmmott/GUI/detail/skia_font_metrics_provider.hpp>
#include <unistd.h>

namespace FredEmmott::GUI {

namespace {

// vcpkg's statically-linked fontconfig is built with a baked-in
// CONFDIR/FONTCONFIG_FILE that doesn't match the host system, so
// FcInitLoadConfigAndFonts() reports "Cannot load default config file:
// No such file: (null)" and Skia's font manager comes back with no
// usable fonts — every glyph then measures inconsistently and FUI's
// FontIcon assertions trip. Point fontconfig at the host's
// /etc/fonts/fonts.conf if it exists; the user can override via the
// FONTCONFIG_FILE env var.
//
// 0 as the third arg to setenv means "don't override an existing value"
// so the user-set env var wins.
void EnsureFontconfigConfig() {
  static const char* const Candidates[] = {
    "/etc/fonts/fonts.conf",
    "/usr/local/etc/fonts/fonts.conf",
  };
  for (const char* const path : Candidates) {
    if (access(path, R_OK) == 0) {
      setenv("FONTCONFIG_FILE", path, /*overwrite=*/0);
      return;
    }
  }
}

// Device extensions we *require*. VK_KHR_swapchain for presentation.
// VK_EXT_external_memory_fd + VK_EXT_external_semaphore_fd are prerequisites
// for the dmabuf import path (ImportedTexture::HandleKind::DmabufFD); they're
// trivially supported on every Mesa / NVIDIA driver so we require them
// upfront rather than make them optional.
constexpr std::array RequiredDeviceExtensions {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
  VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
};

constexpr bool EnableValidation = Config::Debug;

void Check(VkResult r, const char* what) {
  if (r != VK_SUCCESS) {
    throw std::runtime_error(
      std::string {"Vulkan "} + what + " failed: VkResult=" + std::to_string(r));
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
  VkDebugUtilsMessageTypeFlagsEXT,
  const VkDebugUtilsMessengerCallbackDataEXT* data,
  void*) {
  if (severity
      >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::fprintf(stderr, "[Vulkan] %s\n", data->pMessage);
  }
  return VK_FALSE;
}

bool HasInstanceLayer(const char* layer) {
  uint32_t count = 0;
  vkEnumerateInstanceLayerProperties(&count, nullptr);
  std::vector<VkLayerProperties> layers(count);
  vkEnumerateInstanceLayerProperties(&count, layers.data());
  for (const auto& l : layers) {
    if (std::strcmp(l.layerName, layer) == 0) {
      return true;
    }
  }
  return false;
}

bool HasInstanceExtension(const char* ext) {
  uint32_t count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> exts(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count, exts.data());
  for (const auto& e : exts) {
    if (std::strcmp(e.extensionName, ext) == 0) {
      return true;
    }
  }
  return false;
}

}// namespace

// --- FramePainter ---------------------------------------------------------

class SdlSkiaVulkanWindow::FramePainter final : public BasicFramePainter {
 public:
  FramePainter() = delete;
  FramePainter(
    SdlSkiaVulkanWindow* window,
    uint32_t imageIndex,
    VkSemaphore acquireSem,
    sk_sp<SkSurface> surface)
    : mWindow(window),
      mImageIndex(imageIndex),
      mAcquireSem(acquireSem),
      mSurface(std::move(surface)),
      mRenderer(
        SkiaRenderer::NativeDevice {
          .mDPI = {
            .mActual = static_cast<uint64_t>(
              std::lround(window->GetDPIScale() * 96.0)),
            .mNominal = 96,
          },
          .mSkiaContext = window->mSkContext.get(),
        },
        mSurface->getCanvas(),
        /*GPUCompletionFlag=*/nullptr) {
    // Chain acquire → Skia render: tell Skia to wait on the acquire
    // semaphore before any of its submitted GPU commands run.
    // `false` = Skia doesn't take ownership (we reuse the sem next frame).
    auto backendAcquire = GrBackendSemaphores::MakeVk(mAcquireSem);
    mWindow->mSkContext->wait(1, &backendAcquire, /*deleteAfterWait=*/false);
  }

  ~FramePainter() override {
    if (!mWindow || !mSurface) {
      return;
    }
    auto& sync = mWindow->mFrameSync[mImageIndex];

    // Chain Skia render → present: flush the surface with a Vulkan signal
    // semaphore that we then wait on in vkQueuePresentKHR.
    auto presentSem = GrBackendSemaphores::MakeVk(sync.mRenderFinished);
    GrFlushInfo info {};
    info.fNumSemaphores = 1;
    info.fSignalSemaphores = &presentSem;

    // Kick Skia's submission. kPresent transitions the VkImage to
    // PRESENT_SRC_KHR, which is what vkQueuePresentKHR expects.
    mWindow->mSkContext->flush(
      mSurface.get(), SkSurfaces::BackendSurfaceAccess::kPresent, info);
    mWindow->mSkContext->submit();

    const VkPresentInfoKHR present {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &sync.mRenderFinished,
      .swapchainCount = 1,
      .pSwapchains = &mWindow->mSwapchain,
      .pImageIndices = &mImageIndex,
    };
    vkQueuePresentKHR(mWindow->mGraphicsQueue, &present);

    // MVP: brute-force wait-idle so semaphore reuse next frame is trivially
    // safe. Replace with per-frame-in-flight fences so the CPU can pipeline
    // ahead of the GPU (follow-up).
    vkDeviceWaitIdle(mWindow->mDevice);
  }

  Renderer* GetRenderer() noexcept override {
    return &mRenderer;
  }

 private:
  SdlSkiaVulkanWindow* mWindow {nullptr};
  uint32_t mImageIndex {0};
  VkSemaphore mAcquireSem {VK_NULL_HANDLE};
  sk_sp<SkSurface> mSurface;
  SkiaRenderer mRenderer;
};

// --- Lifecycle ------------------------------------------------------------

SdlSkiaVulkanWindow::SdlSkiaVulkanWindow(Options options)
  : SdlWindow(std::move(options)) {
  // Must run before SystemFont::GetFontManager() (called lazily on first
  // text-measure) so fontconfig sees the path on its first FcInit call.
  EnsureFontconfigConfig();

  // FUI's text-measuring code paths (Font::MeasureTextWidth, font metrics
  // queries) call renderer_detail::GetFontMetricsProvider() and throw if
  // SetRenderAPI was never called. Win32Direct3D12GaneshWindow does this
  // in its ctor; mirror it here.
  using namespace renderer_detail;
  if (!HaveRenderAPI(RenderAPI::Skia)) {
    SetRenderAPI(
      RenderAPI::Skia,
      "Skia(Ganesh)+Vulkan",
      std::make_unique<SkiaFontMetricsProvider>());
  }
}

int SdlSkiaVulkanWindow::Run(
  const Options& options,
  const std::function<void(SdlSkiaVulkanWindow&)>& appTick) {
  SdlSkiaVulkanWindow window(options);
  window.SetCancelAction([&] { window.RequestStop(EXIT_SUCCESS); });
  // Pre-create the SDL window so InitializeGraphicsAPI's CreateSurface has
  // something to hand to SDL_Vulkan_CreateSurface on the first BeginFrame.
  window.InitializeWindow();
  if (!window.GetNativeHandle()) {
    return EXIT_FAILURE;
  }
  while (true) {
    window.WaitFrame();
    if (const auto ok = window.BeginFrame(); !ok.has_value()) {
      return ok.error();
    }
    appTick(window);
    if (const auto ec = window.GetExitCode()) {
      return *ec;
    }
    window.EndFrame();
  }
}

SdlSkiaVulkanWindow::~SdlSkiaVulkanWindow() {
  if (mDevice != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(mDevice);
  }
  this->DestroySwapchainResources();
  this->DestroyVulkan();
}

uint64_t SdlSkiaVulkanWindow::GetSDLWindowFlags() const {
  // Inherit base flags (resizable, HiDPI) and add SDL_WINDOW_VULKAN so
  // SDL_Vulkan_CreateSurface() works on the resulting window.
  return SdlWindow::GetSDLWindowFlags() | SDL_WINDOW_VULKAN;
}

void SdlSkiaVulkanWindow::InitializeGraphicsAPI() {
  // Run() pre-creates the SDL window before the first BeginFrame, but
  // the popup path (BeginBasicPopupWindow → BeginFrame) doesn't — it
  // expects InitializeWindow to be called lazily. Do it here so
  // CreateSurface has an SDL_Window* to hand to SDL_Vulkan_CreateSurface.
  if (!this->GetNativeHandle()) {
    this->InitializeWindow();
  }
  this->CreateInstance();
  this->CreateSurface();
  this->PickPhysicalDevice();
  this->CreateDevice();
  this->CreateSwapchain();
  this->CreateSkiaContext();
  this->WrapSwapchainImagesAsSkSurfaces();
}

std::unique_ptr<Window> SdlSkiaVulkanWindow::CreatePopup() const {
  // 240x80 is a placeholder — popups should be sized by their content
  // after layout. Real auto-sizing is a follow-up.
  return std::make_unique<SdlSkiaVulkanWindow>(
    Options {
      .mTitle = "FUI popup",
      .mInitialSize = Size {240, 80},
    });
}

// --- CreateInstance -------------------------------------------------------

void SdlSkiaVulkanWindow::CreateInstance() {
  // SDL tells us which instance extensions are needed to present on this
  // platform (VK_KHR_surface + one of VK_KHR_xlib_surface / _xcb_surface /
  // _wayland_surface, plus portability bits).
  uint32_t sdlExtCount = 0;
  const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
  if (!sdlExts) {
    throw std::runtime_error(
      std::string {"SDL_Vulkan_GetInstanceExtensions failed: "}
      + SDL_GetError());
  }

  std::vector<const char*> extensions(sdlExts, sdlExts + sdlExtCount);
  std::vector<const char*> layers;

  // Validation is opt-in. Only request it if the layer + debug-utils
  // extension are actually installed; otherwise vkCreateInstance fails
  // with VK_ERROR_LAYER_NOT_PRESENT (-6) on systems without the
  // vulkan-validationlayers package. Silent fall-through is fine in
  // release builds.
  bool validation = false;
  if constexpr (EnableValidation) {
    if (
      HasInstanceLayer("VK_LAYER_KHRONOS_validation")
      && HasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
      layers.push_back("VK_LAYER_KHRONOS_validation");
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      validation = true;
    } else {
      std::fprintf(
        stderr,
        "[Vulkan] validation layer not installed; continuing without it. "
        "Install vulkan-validationlayers (Debian/Ubuntu: "
        "vulkan-validationlayers; Fedora: vulkan-validation-layers; "
        "Arch: vulkan-validation-layers) for richer diagnostics.\n");
    }
  }

  const VkApplicationInfo app {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "FUI",
    .applicationVersion = 0,
    .pEngineName = "FredEmmott::GUI",
    .engineVersion = 0,
    .apiVersion = VK_API_VERSION_1_2,
  };

  const VkInstanceCreateInfo create {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app,
    .enabledLayerCount = static_cast<uint32_t>(layers.size()),
    .ppEnabledLayerNames = layers.data(),
    .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data(),
  };
  Check(vkCreateInstance(&create, nullptr, &mInstance), "vkCreateInstance");

  if (validation) {
    const auto createMessenger =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (createMessenger) {
      VkDebugUtilsMessengerCreateInfoEXT info {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity
        = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugCallback,
      };
      createMessenger(mInstance, &info, nullptr, &mDebugMessenger);
    }
  }
}

void SdlSkiaVulkanWindow::CreateSurface() {
  // Cast from Window::NativeHandle (void*) to SDL_Window* — see
  // SdlWindow::GetNativeHandle.
  auto* const sdl = static_cast<SDL_Window*>(this->GetNativeHandle().mValue);
  if (!sdl) {
    throw std::runtime_error(
      "CreateSurface called before SdlWindow::InitializeWindow");
  }
  if (!SDL_Vulkan_CreateSurface(sdl, mInstance, nullptr, &mSurface)) {
    throw std::runtime_error(
      std::string {"SDL_Vulkan_CreateSurface failed: "} + SDL_GetError());
  }
}

// --- PickPhysicalDevice ---------------------------------------------------

void SdlSkiaVulkanWindow::PickPhysicalDevice() {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(mInstance, &count, nullptr);
  if (count == 0) {
    throw std::runtime_error("No Vulkan physical devices");
  }
  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(mInstance, &count, devices.data());

  // Prefer a discrete GPU that has a graphics queue with present support
  // on our surface. Fall back to whichever integrated/software device comes
  // first with those capabilities.
  VkPhysicalDevice fallback = VK_NULL_HANDLE;
  uint32_t fallbackQueue = UINT32_MAX;

  for (const auto dev : devices) {
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qProps(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &qCount, qProps.data());

    for (uint32_t i = 0; i < qCount; ++i) {
      if (!(qProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
        continue;
      }
      VkBool32 present = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, mSurface, &present);
      if (!present) {
        continue;
      }

      VkPhysicalDeviceProperties props {};
      vkGetPhysicalDeviceProperties(dev, &props);
      if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        mPhysicalDevice = dev;
        mGraphicsQueueFamily = i;
        return;
      }
      if (fallback == VK_NULL_HANDLE) {
        fallback = dev;
        fallbackQueue = i;
      }
    }
  }
  if (fallback == VK_NULL_HANDLE) {
    throw std::runtime_error("No Vulkan device with graphics+present queue");
  }
  mPhysicalDevice = fallback;
  mGraphicsQueueFamily = fallbackQueue;
}

// --- CreateDevice ---------------------------------------------------------

void SdlSkiaVulkanWindow::CreateDevice() {
  const float priority = 1.0f;
  const VkDeviceQueueCreateInfo queueCreate {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = mGraphicsQueueFamily,
    .queueCount = 1,
    .pQueuePriorities = &priority,
  };

  // Enable every feature the physical device supports. Skia's GrVkCaps
  // gates render-target formats and draw paths on the feature set; passing
  // a zero-initialised struct (the previous behaviour) made Skia reject
  // BGRA8 render-target wrapping and silently return null.
  //
  // Use the Features2 pNext path rather than VkDeviceCreateInfo::
  // pEnabledFeatures: Vulkan 1.1+ added extended feature blocks that only
  // surface through the pNext chain, and Skia's GrVkCaps prefers that
  // path when set. When pNext carries Features2, pEnabledFeatures must
  // be null.
  vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &mEnabledDeviceFeatures2);

  const VkDeviceCreateInfo create {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &mEnabledDeviceFeatures2,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queueCreate,
    .enabledExtensionCount
    = static_cast<uint32_t>(RequiredDeviceExtensions.size()),
    .ppEnabledExtensionNames = RequiredDeviceExtensions.data(),
    .pEnabledFeatures = nullptr,
  };
  Check(
    vkCreateDevice(mPhysicalDevice, &create, nullptr, &mDevice),
    "vkCreateDevice");
  vkGetDeviceQueue(mDevice, mGraphicsQueueFamily, 0, &mGraphicsQueue);
}

// --- CreateSwapchain ------------------------------------------------------

void SdlSkiaVulkanWindow::CreateSwapchain() {
  VkSurfaceCapabilitiesKHR caps {};
  Check(
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      mPhysicalDevice, mSurface, &caps),
    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

  // Pick format. Skia's GrVkCaps marks VK_FORMAT_B8G8R8A8_UNORM as
  // renderable with kBGRA_8888_SkColorType, but VK_FORMAT_B8G8R8A8_SRGB is
  // texture-wrappable only — not a valid color attachment target. So we
  // *must* request UNORM, even though SRGB+sRGB-colorspace would otherwise
  // be the cleaner gamma path. Pair UNORM with a linear (nullptr)
  // SkColorSpace at WrapBackendRenderTarget time so the (format, color
  // space) pair stays consistent.
  //
  // TODO: gamma is "wrong" in this configuration — colors written by Skia
  // are interpreted as if they were already sRGB-encoded, but Skia itself
  // produces linear values. The visual difference is small for the demo;
  // fix by rendering into an offscreen sRGB SkSurface and blitting to the
  // swapchain.
  uint32_t fmtCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    mPhysicalDevice, mSurface, &fmtCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(fmtCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    mPhysicalDevice, mSurface, &fmtCount, formats.data());
  VkSurfaceFormatKHR chosen = formats.front();
  for (const auto& f : formats) {
    if (
      f.format == VK_FORMAT_B8G8R8A8_UNORM
      && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      chosen = f;
      break;
    }
  }
  mSwapchainFormat = chosen.format;

  // Wayland uses currentExtent={UINT32_MAX, UINT32_MAX} as a sentinel meaning
  // "the client picks the size". On X11 currentExtent is the real window
  // size. Resolve to the SDL window's pixel size when the sentinel is set,
  // then clamp to caps.{min,max}ImageExtent.
  if (
    caps.currentExtent.width == UINT32_MAX
    || caps.currentExtent.height == UINT32_MAX) {
    auto* const sdl = static_cast<SDL_Window*>(this->GetNativeHandle().mValue);
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(sdl, &w, &h);
    mSwapchainExtent.width = std::clamp<uint32_t>(
      static_cast<uint32_t>(std::max(w, 1)),
      caps.minImageExtent.width,
      caps.maxImageExtent.width);
    mSwapchainExtent.height = std::clamp<uint32_t>(
      static_cast<uint32_t>(std::max(h, 1)),
      caps.minImageExtent.height,
      caps.maxImageExtent.height);
  } else {
    mSwapchainExtent = caps.currentExtent;
  }

  const uint32_t imageCount
    = std::min<uint32_t>(
      caps.minImageCount + 1,
      caps.maxImageCount ? caps.maxImageCount : caps.minImageCount + 1);

  // Image usage: COLOR_ATTACHMENT is the minimum for swapchain RTs;
  // TRANSFER_DST lets us vkCmdClearColorImage; TRANSFER_SRC + SAMPLED let
  // Skia copy / sample from the image during its draw paths
  // (GrVkRenderTarget::MakeWrappedRenderTarget rejects images that don't
  // expose these). Both are guaranteed to be supported on swapchain
  // images per the Vulkan spec when reported in
  // VkSurfaceCapabilitiesKHR::supportedUsageFlags — which we don't check
  // here, but BGRA8_UNORM swapchains support all 4 on every Mesa/NVIDIA
  // driver in practice.
  constexpr VkImageUsageFlags SwapchainUsage =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    | VK_IMAGE_USAGE_SAMPLED_BIT;

  const VkSwapchainCreateInfoKHR create {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = mSurface,
    .minImageCount = imageCount,
    .imageFormat = chosen.format,
    .imageColorSpace = chosen.colorSpace,
    .imageExtent = mSwapchainExtent,
    .imageArrayLayers = 1,
    .imageUsage = SwapchainUsage,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = caps.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = VK_PRESENT_MODE_FIFO_KHR,// v-sync, always supported
    .clipped = VK_TRUE,
  };
  Check(
    vkCreateSwapchainKHR(mDevice, &create, nullptr, &mSwapchain),
    "vkCreateSwapchainKHR");

  uint32_t gotImages = 0;
  vkGetSwapchainImagesKHR(mDevice, mSwapchain, &gotImages, nullptr);
  mSwapchainImages.resize(gotImages);
  vkGetSwapchainImagesKHR(
    mDevice, mSwapchain, &gotImages, mSwapchainImages.data());

  mFrameSync.resize(gotImages);
  const VkSemaphoreCreateInfo semCI {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  for (auto& s : mFrameSync) {
    Check(
      vkCreateSemaphore(mDevice, &semCI, nullptr, &s.mImageAvailable),
      "vkCreateSemaphore");
    Check(
      vkCreateSemaphore(mDevice, &semCI, nullptr, &s.mRenderFinished),
      "vkCreateSemaphore");
  }
}

// --- CreateSkiaContext ----------------------------------------------------

void SdlSkiaVulkanWindow::CreateSkiaContext() {
  // Skia needs function pointers for the Vulkan entry points via its own
  // GrVkGetProc signature (a std::function that dispatches instance or
  // device procs depending on arguments). The small lambda wraps our
  // vkGetInstanceProcAddr / vkGetDeviceProcAddr.
  skgpu::VulkanGetProc getProc = [this](
                                   const char* name,
                                   VkInstance instance,
                                   VkDevice device) {
    if (device != VK_NULL_HANDLE) {
      return vkGetDeviceProcAddr(device, name);
    }
    return vkGetInstanceProcAddr(instance, name);
  };

  skgpu::VulkanExtensions vkExtensions;
  vkExtensions.init(
    getProc,
    mInstance,
    mPhysicalDevice,
    /*instanceExtCount=*/0,
    /*instanceExts=*/nullptr,
    /*deviceExtCount=*/
    static_cast<uint32_t>(RequiredDeviceExtensions.size()),
    /*deviceExts=*/RequiredDeviceExtensions.data());

  skgpu::VulkanBackendContext vkCtx {};
  vkCtx.fInstance = mInstance;
  vkCtx.fPhysicalDevice = mPhysicalDevice;
  vkCtx.fDevice = mDevice;
  vkCtx.fQueue = mGraphicsQueue;
  vkCtx.fGraphicsQueueIndex = mGraphicsQueueFamily;
  vkCtx.fMaxAPIVersion = VK_API_VERSION_1_2;
  vkCtx.fVkExtensions = &vkExtensions;
  vkCtx.fGetProc = getProc;
  // Skia: "If fDeviceFeatures2 is not null then we ignore fDeviceFeatures.
  // If both are null we will assume no features are enabled." We populated
  // Features2 at vkCreateDevice via pNext, so point Skia at the same
  // struct here.
  vkCtx.fDeviceFeatures2 = &mEnabledDeviceFeatures2;

  mSkContext = GrDirectContexts::MakeVulkan(vkCtx);
  if (!mSkContext) {
    throw std::runtime_error("GrDirectContexts::MakeVulkan returned null");
  }
}

// --- WrapSwapchainImagesAsSkSurfaces --------------------------------------

void SdlSkiaVulkanWindow::WrapSwapchainImagesAsSkSurfaces() {
  // One-time self-test before we wrap anything. Logs Skia's preferred
  // VkFormat for kBGRA_8888 + renderable; if it differs from what the
  // swapchain handed us, we have a format mismatch that wrap silently
  // rejects.
  static std::once_flag selfTest;
  std::call_once(selfTest, [this] {
    const auto bf = mSkContext->defaultBackendFormat(
      kBGRA_8888_SkColorType, skgpu::Renderable::kYes);
    VkFormat preferredFmt = VK_FORMAT_UNDEFINED;
    GrBackendFormats::AsVkFormat(bf, &preferredFmt);
    std::fprintf(
      stderr,
      "[FUI] Skia self-test:\n"
      "      colorTypeSupportedAsSurface(BGRA8888) = %d\n"
      "      maxSurfaceSampleCountForColorType(BGRA8888) = %d\n"
      "      maxRenderTargetSize = %d\n"
      "      defaultBackendFormat(BGRA8888,kYes).VkFormat = %d\n"
      "      mSwapchainFormat = %d\n",
      mSkContext->colorTypeSupportedAsSurface(kBGRA_8888_SkColorType),
      mSkContext->maxSurfaceSampleCountForColorType(kBGRA_8888_SkColorType),
      mSkContext->maxRenderTargetSize(),
      static_cast<int>(preferredFmt),
      static_cast<int>(mSwapchainFormat));
  });

  mSwapchainSurfaces.resize(mSwapchainImages.size());
  for (size_t i = 0; i < mSwapchainImages.size(); ++i) {
    GrVkImageInfo imageInfo {};
    imageInfo.fImage = mSwapchainImages[i];
    imageInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.fFormat = mSwapchainFormat;
    // Must match the imageUsage we asked vkCreateSwapchainKHR for.
    imageInfo.fImageUsageFlags
      = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
      | VK_IMAGE_USAGE_TRANSFER_DST_BIT
      | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
      | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.fSampleCount = 1;
    imageInfo.fLevelCount = 1;

    const auto rt = GrBackendRenderTargets::MakeVk(
      static_cast<int>(mSwapchainExtent.width),
      static_cast<int>(mSwapchainExtent.height),
      imageInfo);

    // VK_FORMAT_B8G8R8A8_SRGB → app draws linear, GPU sRGB-encodes on write,
    // SkColorSpace must be sRGB. VK_FORMAT_B8G8R8A8_UNORM → no auto-encode,
    // SkColorSpace must be linear (nullptr). Mismatched pairs make
    // WrapBackendRenderTarget return null.
    sk_sp<SkColorSpace> skColorSpace =
      (mSwapchainFormat == VK_FORMAT_B8G8R8A8_SRGB)
      ? SkColorSpace::MakeSRGB()
      : nullptr;
    mSwapchainSurfaces[i] = SkSurfaces::WrapBackendRenderTarget(
      mSkContext.get(),
      rt,
      kTopLeft_GrSurfaceOrigin,
      kBGRA_8888_SkColorType,
      std::move(skColorSpace),
      /*props=*/nullptr);
    if (!mSwapchainSurfaces[i]) {
      std::fprintf(
        stderr,
        "[FUI] WrapBackendRenderTarget(image=%zu) returned null:\n"
        "      swapchain VkFormat = %d (%s)\n"
        "      extent = %ux%u\n"
        "      SkColorType = kBGRA_8888\n"
        "      SkColorSpace = %s\n"
        "      GrBackendRenderTarget.isValid() = %d\n"
        "      GrDirectContext->abandoned() = %d\n",
        i,
        static_cast<int>(mSwapchainFormat),
        (mSwapchainFormat == VK_FORMAT_B8G8R8A8_SRGB)   ? "B8G8R8A8_SRGB"
          : (mSwapchainFormat == VK_FORMAT_B8G8R8A8_UNORM) ? "B8G8R8A8_UNORM"
                                                           : "other",
        mSwapchainExtent.width,
        mSwapchainExtent.height,
        (mSwapchainFormat == VK_FORMAT_B8G8R8A8_SRGB) ? "sRGB" : "linear",
        rt.isValid() ? 1 : 0,
        mSkContext->abandoned() ? 1 : 0);
      throw std::runtime_error(
        "SkSurfaces::WrapBackendRenderTarget returned null");
    }
  }
}

// --- Resize ---------------------------------------------------------------

void SdlSkiaVulkanWindow::ResizeBackend() {
  // Called by SdlWindow::ResizeIfNeeded after popup auto-fit (which
  // may have called SDL_SetWindowSize). Re-query SDL pixel size and
  // rebuild the swapchain in the same frame if it changed.
  auto* const sdl = static_cast<SDL_Window*>(this->GetNativeHandle().mValue);
  if (!sdl || !mDevice) {
    return;
  }
  int w = 0, h = 0;
  SDL_GetWindowSizeInPixels(sdl, &w, &h);
  if (
    static_cast<uint32_t>(w) == mSwapchainExtent.width
    && static_cast<uint32_t>(h) == mSwapchainExtent.height) {
    return;
  }
  vkDeviceWaitIdle(mDevice);
  this->DestroySwapchainResources();
  this->CreateSwapchain();
  this->WrapSwapchainImagesAsSkSurfaces();
}

// --- GetFramePainter ------------------------------------------------------

std::unique_ptr<Window::BasicFramePainter>
SdlSkiaVulkanWindow::GetFramePainter(uint8_t frameIndex) {
  // Pick any semaphore as "will be signaled by acquire". Pass it to
  // vkAcquireNextImageKHR; the returned imageIndex tells us which image
  // that semaphore attaches to. We'll then use per-image mRenderFinished
  // for the present wait. vkDeviceWaitIdle in FramePainter::~FramePainter
  // guarantees this semaphore is unsignaled when reused next frame.
  VkSemaphore acquireSem
    = mFrameSync[mCurrentSwapchainImage].mImageAvailable;

  uint32_t imageIndex = 0;
  const auto acq = vkAcquireNextImageKHR(
    mDevice,
    mSwapchain,
    UINT64_MAX,
    acquireSem,
    VK_NULL_HANDLE,
    &imageIndex);
  if (acq == VK_ERROR_OUT_OF_DATE_KHR || acq == VK_SUBOPTIMAL_KHR) {
    this->ResizeIfNeeded();
    return GetFramePainter(frameIndex);// single retry after rebuild
  }
  Check(acq, "vkAcquireNextImageKHR");

  mCurrentSwapchainImage = imageIndex;
  return std::unique_ptr<BasicFramePainter> {
    new FramePainter(this, imageIndex, acquireSem, mSwapchainSurfaces[imageIndex])};
}

// --- Teardown -------------------------------------------------------------

void SdlSkiaVulkanWindow::DestroySwapchainResources() {
  if (mDevice == VK_NULL_HANDLE) {
    return;
  }
  mSwapchainSurfaces.clear();
  for (auto& s : mFrameSync) {
    if (s.mImageAvailable) vkDestroySemaphore(mDevice, s.mImageAvailable, nullptr);
    if (s.mRenderFinished) vkDestroySemaphore(mDevice, s.mRenderFinished, nullptr);
  }
  mFrameSync.clear();
  for (auto iv : mSwapchainImageViews) {
    vkDestroyImageView(mDevice, iv, nullptr);
  }
  mSwapchainImageViews.clear();
  mSwapchainImages.clear();
  if (mSwapchain) {
    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
    mSwapchain = VK_NULL_HANDLE;
  }
}

void SdlSkiaVulkanWindow::DestroyVulkan() {
  mSkContext.reset();
  if (mDevice) {
    vkDestroyDevice(mDevice, nullptr);
    mDevice = VK_NULL_HANDLE;
  }
  if (mSurface) {
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    mSurface = VK_NULL_HANDLE;
  }
  if (mDebugMessenger) {
    const auto destroyMessenger =
      reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (destroyMessenger) {
      destroyMessenger(mInstance, mDebugMessenger, nullptr);
    }
    mDebugMessenger = VK_NULL_HANDLE;
  }
  if (mInstance) {
    vkDestroyInstance(mInstance, nullptr);
    mInstance = VK_NULL_HANDLE;
  }
}

}// namespace FredEmmott::GUI
