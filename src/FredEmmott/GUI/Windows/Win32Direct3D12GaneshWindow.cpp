// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Win32Direct3D12GaneshWindow.hpp"

#include <Windowsx.h>
#include <d3d11_4.h>
#include <dwmapi.h>
#include <skia/core/SkColorSpace.h>
#include <wil/win32_helpers.h>

#include <FredEmmott/GUI/SkiaRenderer.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>
#include <chrono>
#include <filesystem>
#include <print>
#include <thread>

#include "FredEmmott/GUI/detail/renderer_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail/CopySoftwareBitmap.hpp"

#if __has_include(<skia/gpu/ganesh/GrDirectContext.h>)
#include <skia/gpu/ganesh/GrBackendSemaphore.h>
#include <skia/gpu/ganesh/GrBackendSurface.h>
#include <skia/gpu/ganesh/GrDirectContext.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>
#include <skia/gpu/ganesh/d3d/GrD3DBackendContext.h>
#else
#include <skia/gpu/GrBackendSemaphore.h>
#include <skia/gpu/GrBackendSurface.h>
#include <skia/gpu/GrDirectContext.h>
#include <skia/gpu/d3d/GrD3DBackendContext.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>
#endif

namespace FredEmmott::GUI {
namespace {
using namespace win32_detail;

struct SkiaFontMetricsProvider final : renderer_detail::FontMetricsProvider {
  ~SkiaFontMetricsProvider() override = default;

  float MeasureTextWidth(const Font& font, const std::string_view text)
    const override {
    if (!font) {
      return std::numeric_limits<float>::quiet_NaN();
    }
    const auto it = font.as<SkFont>();
    return it.measureText(text.data(), text.size(), SkTextEncoding::kUTF8);
  }

  Font::Metrics GetFontMetrics(const Font& font) const override {
    using namespace font_detail;
    const auto it = font.as<SkFont>();
    SkFontMetrics pt {};
    const auto lineSpacingPt = it.getMetrics(&pt);
    return {
      .mSize = it.getSize(),
      .mLineSpacing = lineSpacingPt,
      .mAscent = pt.fAscent,
      .mDescent = pt.fDescent,
    };
  }
};

void ConfigureD3DDebugLayer(
  [[maybe_unused]] const wil::com_ptr<ID3D12Device>& device) {
  if constexpr (Config::Debug) {
    const auto infoQueue = device.try_query<ID3D12InfoQueue1>();
    if (!infoQueue) {
      return;
    }

    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

    // Skia internally triggers this; explicitly suppress it so we can
    // keep breaking on everything WARNING or above
    D3D12_MESSAGE_ID skiaIssues[] = {
      D3D12_MESSAGE_ID_DESCRIPTOR_HEAP_NOT_SHADER_VISIBLE,
      D3D12_MESSAGE_ID_CREATE_SAMPLER_COMPARISON_FUNC_IGNORED,
    };
    for (const auto id: skiaIssues) {
      infoQueue->SetBreakOnID(id, false);
    }

    D3D12_MESSAGE_SEVERITY allowSeverities[] = {
      D3D12_MESSAGE_SEVERITY_WARNING,
      D3D12_MESSAGE_SEVERITY_ERROR,
      D3D12_MESSAGE_SEVERITY_CORRUPTION,
    };

    D3D12_INFO_QUEUE_FILTER filter {
      .AllowList = D3D12_INFO_QUEUE_FILTER_DESC {
        .NumSeverities = std::size(allowSeverities),
        .pSeverityList = allowSeverities,
      },
      .DenyList = D3D12_INFO_QUEUE_FILTER_DESC {
        .NumIDs = std::size(skiaIssues),
        .pIDList = skiaIssues,
      },
    };
    CheckHResult(infoQueue->PushStorageFilter(&filter));
  }
}

struct D3D12CompletionFlag : GPUCompletionFlag {
  D3D12CompletionFlag(ID3D12Fence* const fence, const uint64_t fenceValue)
    : mEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
      mFence(fence),
      mFenceValue(fenceValue) {
    FUI_ASSERT(fenceValue > 0);
    CheckHResult(mFence->SetEventOnCompletion(fenceValue, mEvent.get()));
  }
  ~D3D12CompletionFlag() override = default;

  bool IsComplete() const override {
    return mFence->GetCompletedValue() >= mFenceValue;
  }

  void Wait() const override {
    if (!IsComplete()) {
      WaitForSingleObject(mEvent.get(), INFINITE);
    }
  }

 private:
  wil::unique_event mEvent;
  wil::com_ptr<ID3D12Fence> mFence {nullptr};
  uint64_t mFenceValue {};
};

}// namespace

class Win32Direct3D12GaneshWindow::FramePainter final
  : public BasicFramePainter {
 public:
  FramePainter() = delete;
  FramePainter(Win32Direct3D12GaneshWindow* window, uint8_t frameIndex)
    : mWindow(window),
      mFrameIndex(frameIndex),
      mRenderer(
        SkiaRenderer::NativeDevice {
          {
            .mActual = static_cast<uint64_t>(
              std::lround(window->GetDPIScale() * USER_DEFAULT_SCREEN_DPI)),
            .mNominal = USER_DEFAULT_SCREEN_DPI,
          },
          window->mD3DDevice.get(),
          window->mD3DCommandQueue.get(),
          window->mSkContext.get()},
        window->mFrame.mSkSurface->getCanvas(),
        std::make_shared<D3D12CompletionFlag>(
          window->mD3DFence.get(),
          window->mFrame.mFenceValue)) {
    mWindow->BeforePaintFrame(frameIndex);
  }

  ~FramePainter() override {
    mWindow->AfterPaintFrame(mFrameIndex);
  }

  Renderer* GetRenderer() noexcept override {
    return &mRenderer;
  }

 private:
  Win32Direct3D12GaneshWindow* mWindow {nullptr};
  uint8_t mFrameIndex {};
  SkiaRenderer mRenderer;
};

struct Win32Direct3D12GaneshWindow::SharedResources {
  wil::com_ptr<IDXGIAdapter1> mDXGIAdapter;
  wil::com_ptr<ID3D12Device> mD3DDevice;
  wil::com_ptr<ID3D12CommandQueue> mD3DCommandQueue;
  sk_sp<GrDirectContext> mSkContext;

  wil::com_ptr<ID3D11Device5> mD3D11Device;
  wil::com_ptr<ID3D11DeviceContext4> mD3D11DeviceContext;

  static std::shared_ptr<SharedResources> Get(IDXGIFactory4* dxgiFactory);
};

std::shared_ptr<Win32Direct3D12GaneshWindow::SharedResources>
Win32Direct3D12GaneshWindow::SharedResources::Get(IDXGIFactory4* dxgiFactory) {
  thread_local std::weak_ptr<Win32Direct3D12GaneshWindow::SharedResources>
    sShared;
  if (const auto ret = sShared.lock()) {
    return ret;
  }

  auto ret = std::shared_ptr<SharedResources>(new SharedResources());

  if constexpr (Config::Debug) {
    wil::com_ptr<ID3D12Debug5> d3d12Debug;
    D3D12GetDebugInterface(IID_PPV_ARGS(d3d12Debug.put()));
    if (d3d12Debug) {
      d3d12Debug->EnableDebugLayer();
      d3d12Debug->SetEnableAutoName(true);
    }
  }

  CheckHResult(dxgiFactory->EnumAdapters1(0, ret->mDXGIAdapter.put()));

  D3D_FEATURE_LEVEL featureLevel {D3D_FEATURE_LEVEL_11_0};
  CheckHResult(D3D12CreateDevice(
    ret->mDXGIAdapter.get(),
    featureLevel,
    IID_PPV_ARGS(ret->mD3DDevice.put())));
  ConfigureD3DDebugLayer(ret->mD3DDevice);

  {
    D3D12_COMMAND_QUEUE_DESC desc {
      .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
      .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
    };
    CheckHResult(ret->mD3DDevice->CreateCommandQueue(
      &desc, IID_PPV_ARGS(ret->mD3DCommandQueue.put())));
  }

  GrD3DBackendContext skiaD3DContext {};
  skiaD3DContext.fAdapter.retain(ret->mDXGIAdapter.get());
  skiaD3DContext.fDevice.retain(ret->mD3DDevice.get());
  skiaD3DContext.fQueue.retain(ret->mD3DCommandQueue.get());
  ret->mSkContext = GrDirectContext::MakeDirect3D(skiaD3DContext);

  UINT d3d11Flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  if constexpr (Config::Debug) {
    d3d11Flags |= D3D11_CREATE_DEVICE_DEBUG;
  }

  {
    wil::com_ptr<ID3D11Device> device;
    wil::com_ptr<ID3D11DeviceContext> context;
    CheckHResult(D3D11CreateDevice(
      ret->mDXGIAdapter.get(),
      D3D_DRIVER_TYPE_UNKNOWN,
      nullptr,
      d3d11Flags,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      device.put(),
      nullptr,
      context.put()));
    device.query_to(ret->mD3D11Device.put());
    context.query_to(ret->mD3D11DeviceContext.put());
  }

  sShared = ret;
  return ret;
}

void Win32Direct3D12GaneshWindow::InitializeGraphicsAPI() {
  if (mSharedResources) {
    return;
  }
  this->InitializeD3D();
  this->InitializeSkia();
}

void Win32Direct3D12GaneshWindow::InitializeD3D() {
  mSharedResources = SharedResources::Get(this->GetDXGIFactory());
  mDXGIAdapter = mSharedResources->mDXGIAdapter;
  mD3DDevice = mSharedResources->mD3DDevice;
  mD3DCommandQueue = mSharedResources->mD3DCommandQueue;

  CheckHResult(mD3DDevice->CreateFence(
    0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mD3DFence.put())));

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc {
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
      .NumDescriptors = SwapChainLength,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };
    CheckHResult(
      mD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mD3DRTVHeap.put())));
  }

  {
    D3D12_DESCRIPTOR_HEAP_DESC desc {
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
      .NumDescriptors = 1,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
    };
    CheckHResult(
      mD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mD3DSRVHeap.put())));
  }
}

void Win32Direct3D12GaneshWindow::InitializeSkia() {
  mSkContext = mSharedResources->mSkContext;
  FUI_ASSERT(mSkContext);
}

Win32Direct3D12GaneshWindow::Win32Direct3D12GaneshWindow(
  HINSTANCE instance,
  UINT showCommand,
  const Options& options)
  : Win32Window(instance, showCommand, options) {
  using namespace renderer_detail;
  if (HaveRenderAPI(RenderAPI::Skia)) {
    return;
  }
  SetRenderAPI(
    RenderAPI::Skia,
    "Skia(Ganesh)+D3D12",
    std::make_unique<SkiaFontMetricsProvider>());
}

Win32Direct3D12GaneshWindow::~Win32Direct3D12GaneshWindow() {
  this->DestroyWindow();
}

IUnknown* Win32Direct3D12GaneshWindow::GetGPUDeviceForComposition() const {
  return mSharedResources->mD3D11Device.get();
}

void Win32Direct3D12GaneshWindow::CreateRenderTargets() {
  // We now only have a single buffer that D3D12 writes to, but let's keep the
  // math here just in case The multi-item swapchain is now used by d3d11
  static constexpr auto d3d12FrameIndex = 0;
  const auto rtvStart = mD3DRTVHeap->GetCPUDescriptorHandleForHeapStart();
  const auto rtvStep = mD3DDevice->GetDescriptorHandleIncrementSize(
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  const auto swapchain = GetSwapChain();

  CheckHResult(
    swapchain->GetBuffer(0, IID_PPV_ARGS(mFrame.mD3D11SwapChainTexture.put())));
  {
    D3D11_TEXTURE2D_DESC desc {};
    mFrame.mD3D11SwapChainTexture->GetDesc(&desc);
    desc.MiscFlags &= ~D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    desc.MiscFlags
      |= D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    CheckHResult(mSharedResources->mD3D11Device->CreateTexture2D(
      &desc, nullptr, mFrame.mD3D11InteropTexture.put()));
  }

  CheckHResult(
    mFrame.mD3D11InteropTexture.query<IDXGIResource1>()->CreateSharedHandle(
      nullptr,
      DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
      nullptr,
      mFrame.mInteropHandle.put()));

  CheckHResult(mSharedResources->mD3D11Device->CreateFence(
    0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(mInteropFence.mD3D11Fence.put())));
  CheckHResult(mInteropFence.mD3D11Fence->CreateSharedHandle(
    nullptr, GENERIC_ALL, nullptr, mInteropFence.mHandle.put()));

  CheckHResult(mD3DDevice->OpenSharedHandle(
    mFrame.mInteropHandle.get(), IID_PPV_ARGS(mFrame.mRenderTarget.put())));
  CheckHResult(mD3DDevice->OpenSharedHandle(
    mInteropFence.mHandle.get(),
    IID_PPV_ARGS(mInteropFence.mD3D12Fence.put())));

  mFrame.mRenderTarget->SetName(L"FredEmmott::GUI Skia RenderTarget");
  mFrame.mRenderTargetView = rtvStart;
  mFrame.mRenderTargetView.ptr += d3d12FrameIndex * rtvStep;
  mD3DDevice->CreateRenderTargetView(
    mFrame.mRenderTarget.get(), nullptr, mFrame.mRenderTargetView);

  DXGI_SWAP_CHAIN_DESC1 desc;
  swapchain->GetDesc1(&desc);

  const GrD3DTextureResourceInfo backBufferInfo(
    mFrame.mRenderTarget.get(),
    {},
    D3D12_RESOURCE_STATE_COMMON,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    1,
    1,
    0);
  GrBackendRenderTarget backBufferRT(
    static_cast<int>(desc.Width),
    static_cast<int>(desc.Height),
    backBufferInfo);
  mFrame.mSkSurface = SkSurfaces::WrapBackendRenderTarget(
    mSkContext.get(),
    backBufferRT,
    {},
    kRGBA_8888_SkColorType,
    nullptr,
    nullptr);
}

void Win32Direct3D12GaneshWindow::AfterPaintFrame(
  [[maybe_unused]] const uint8_t frameIndex) {
  FUI_ASSERT(mFrame.mFenceValue > 0);
  FUI_ASSERT(mFrame.mFenceValue == mFenceValue);

  GrD3DFenceInfo fenceInfo {};
  fenceInfo.fFence.retain(mD3DFence.get());
  fenceInfo.fValue = mFrame.mFenceValue;
  GrBackendSemaphore flushSemaphore;
  flushSemaphore.initDirect3D(fenceInfo);

  mSkContext->flush(
    mFrame.mSkSurface.get(),
    SkSurfaces::BackendSurfaceAccess::kPresent,
    GrFlushInfo {
      .fNumSemaphores = 1,
      .fSignalSemaphores = &flushSemaphore,
    });
  mSkContext->submit(GrSyncCpu::kNo);

  const auto interopFenceValue = ++mInteropFence.mValue;
  CheckHResult(mD3DCommandQueue->Signal(
    mInteropFence.mD3D12Fence.get(), interopFenceValue));

  const auto d3d11 = mSharedResources->mD3D11DeviceContext.get();
  CheckHResult(d3d11->Wait(mInteropFence.mD3D11Fence.get(), interopFenceValue));
  d3d11->CopySubresourceRegion(
    mFrame.mD3D11SwapChainTexture.get(),
    0,
    0,
    0,
    0,
    mFrame.mD3D11InteropTexture.get(),
    0,
    nullptr);

  CheckHResult(GetSwapChain()->Present(0, 0));
}

void Win32Direct3D12GaneshWindow::CleanupFrameContexts() {
  mSkContext->flushAndSubmit(GrSyncCpu::kYes);

  const auto fenceValue = ++mFenceValue;
  FUI_ASSERT(fenceValue > 0);
  CheckHResult(mD3DCommandQueue->Signal(mD3DFence.get(), fenceValue));
  CheckHResult(mD3DFence->SetEventOnCompletion(fenceValue, mFenceEvent.get()));
  WaitForSingleObject(mFenceEvent.get(), INFINITE);

  mFrame = {};
}

std::unique_ptr<Win32Window> Win32Direct3D12GaneshWindow::CreatePopup(
  HINSTANCE instance,
  int showCommand,
  const Options& options) const {
  return std::make_unique<Win32Direct3D12GaneshWindow>(
    instance, showCommand, options);
}

void Win32Direct3D12GaneshWindow::CopySoftwareBitmap(
  IDXGISurface* dest,
  const BasicPoint<uint32_t>& destOffset,
  const void* inputData,
  const BasicSize<uint32_t>& inputSize,
  const uint32_t inputStride) {
  win32_detail::CopySoftwareBitmap(
    mSharedResources->mD3D11Device.get(),
    mSharedResources->mD3D11DeviceContext.get(),
    dest,
    destOffset,
    inputData,
    inputSize,
    inputStride);
}

std::unique_ptr<Win32Direct3D12GaneshWindow::BasicFramePainter>
Win32Direct3D12GaneshWindow::GetFramePainter(uint8_t frameIndex) {
  auto& frame = mFrame;

  if (frame.mFenceValue) {
    CheckHResult(
      mD3DFence->SetEventOnCompletion(frame.mFenceValue, mFenceEvent.get()));
    WaitForSingleObject(mFenceEvent.get(), INFINITE);
  }
  frame.mFenceValue = ++mFenceValue;

  return std::unique_ptr<BasicFramePainter> {
    new FramePainter(this, frameIndex)};
}

void Win32Direct3D12GaneshWindow::BeforePaintFrame(
  [[maybe_unused]] uint8_t frameIndex) {}

}// namespace FredEmmott::GUI
