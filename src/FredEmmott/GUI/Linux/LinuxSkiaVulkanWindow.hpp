// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux `Window` implementation: Skia Ganesh backed by Vulkan. Sits on top
// of LinuxWindow (SDL3 windowing/input) and fills in the render-side pure
// virtuals: InitializeGraphicsAPI, GetFramePainter, ResizeBackend, CreatePopup.
//
// Parallel to Windows/Win32Direct3D12GaneshWindow (Skia Ganesh on D3D12).
// Enables VK_EXT_external_memory_fd + VK_EXT_external_semaphore_fd on the
// device at creation — prerequisites for the dmabuf import path
// (OpenKneeboard's ink-layer texture lands via dmabuf-fd).
#pragma once

#include <vulkan/vulkan.h>

#include <skia/core/SkSurface.h>
#include <skia/gpu/ganesh/GrDirectContext.h>

#include <functional>
#include <memory>
#include <vector>

#include "LinuxWindow.hpp"

namespace FredEmmott::GUI {

class LinuxSkiaVulkanWindow final : public LinuxWindow {
 public:
  explicit LinuxSkiaVulkanWindow(Options options);
  ~LinuxSkiaVulkanWindow() override;

  // Static driver: construct, run FUI frame loop with `appTick`, return
  // exit code. Parallels Win32Window::WinMain.
  static int Run(
    const Options& options,
    const std::function<void(LinuxSkiaVulkanWindow&)>& appTick);

 public:
  // Override of LinuxWindow::CreatePopup (which returns null because the
  // base LinuxWindow has no renderer). Returns a fresh
  // LinuxSkiaVulkanWindow with its own VkInstance/Device/Swapchain;
  // SetParent + SetInitialPositionInNativeCoords + SetIsToolTip then
  // configure it as an SDL popup before its first BeginFrame.
  [[nodiscard]] std::unique_ptr<Window> CreatePopup() const override;

 protected:
  uint64_t GetSDLWindowFlags() const override;
  void InitializeGraphicsAPI() override;
  void ResizeBackend() override;
  std::unique_ptr<BasicFramePainter> GetFramePainter(uint8_t frameIndex) override;

 private:
  class FramePainter;

  // --- Vulkan instance / device ---
  VkInstance mInstance {VK_NULL_HANDLE};
  VkDebugUtilsMessengerEXT mDebugMessenger {VK_NULL_HANDLE};
  VkPhysicalDevice mPhysicalDevice {VK_NULL_HANDLE};
  VkDevice mDevice {VK_NULL_HANDLE};
  uint32_t mGraphicsQueueFamily {UINT32_MAX};
  VkQueue mGraphicsQueue {VK_NULL_HANDLE};
  // Enabled at vkCreateDevice and pointed at from skgpu::VulkanBackendContext
  // so Skia sees the same feature set the device was created with. Skia's
  // GrVkCaps treats a null fDeviceFeatures2 as "no features available" and
  // then rejects most render-target formats — that path returns a null
  // SkSurface from WrapBackendRenderTarget without any error log. We use
  // the Features2 pNext-capable variant rather than plain Features because
  // Vulkan 1.1+ feature queries flow through the pNext chain and Skia's
  // GrVkCaps prefers that when present.
  VkPhysicalDeviceFeatures2 mEnabledDeviceFeatures2 {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
  };

  // --- Presentation surface + swapchain ---
  VkSurfaceKHR mSurface {VK_NULL_HANDLE};
  VkSwapchainKHR mSwapchain {VK_NULL_HANDLE};
  VkFormat mSwapchainFormat {VK_FORMAT_UNDEFINED};
  VkExtent2D mSwapchainExtent {};
  std::vector<VkImage> mSwapchainImages;
  std::vector<VkImageView> mSwapchainImageViews;
  std::vector<sk_sp<SkSurface>> mSwapchainSurfaces;

  // --- Per-frame sync ---
  //
  // One pair per swapchain image. Back-pressure from VK_PRESENT_MODE_FIFO_KHR
  // (via blocking vkAcquireNextImageKHR) throttles the frame rate — no host
  // fence needed for the current MVP. Proper in-flight fences (so the CPU
  // can pipeline ahead of the GPU) are a follow-up.
  struct FrameSync {
    VkSemaphore mImageAvailable {VK_NULL_HANDLE};
    VkSemaphore mRenderFinished {VK_NULL_HANDLE};
  };
  std::vector<FrameSync> mFrameSync;
  uint32_t mCurrentSwapchainImage {0};

  // --- Skia context ---
  sk_sp<GrDirectContext> mSkContext;

  // --- Lifecycle helpers ---
  void CreateInstance();
  void PickPhysicalDevice();
  void CreateDevice();
  void CreateSurface();
  void CreateSwapchain();
  void CreateSkiaContext();
  void WrapSwapchainImagesAsSkSurfaces();
  void DestroySwapchainResources();
  void DestroyVulkan();
};

}// namespace FredEmmott::GUI
