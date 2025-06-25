// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Win32Direct3D12GaneshWindow.hpp"

#include <Windowsx.h>
#include <dwmapi.h>
#include <skia/core/SkColorSpace.h>
#include <skia/gpu/GrBackendSemaphore.h>
#include <skia/gpu/GrBackendSurface.h>
#include <skia/gpu/GrDirectContext.h>
#include <skia/gpu/d3d/GrD3DBackendContext.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>
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

namespace FredEmmott::GUI {
namespace {
using namespace win32_detail;

struct SkiaFontMetricsProvider final : renderer_detail::FontMetricsProvider {
  ~SkiaFontMetricsProvider() override = default;

  float MeasureTextWidth(const Font& font, const std::string_view text)
    const override {
    const auto it = font.as<SkFont>();
    return font_detail::PointsToPixels(
      it.measureText(text.data(), text.size(), SkTextEncoding::kUTF8));
  }
  Font::Metrics GetFontMetrics(const Font& font) const override {
    using namespace font_detail;
    const auto it = font.as<SkFont>();
    SkFontMetrics pt {};
    const auto lineSpacingPt = PointsToPixels(it.getMetrics(&pt));
    return {
      .mSize = PointsToPixels(it.getSize()),
      .mLineSpacing = PointsToPixels(lineSpacingPt),
      .mDescent = PointsToPixels(pt.fDescent),
    };
  }
};

void ConfigureD3DDebugLayer(const wil::com_ptr<ID3D12Device>& device) {
#ifndef NDEBUG
  auto infoQueue = device.try_query<ID3D12InfoQueue1>();
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
#endif
}
}// namespace

class Win32Direct3D12GaneshWindow::FramePainter final
  : public BasicFramePainter {
 public:
  FramePainter() = delete;
  FramePainter(Win32Direct3D12GaneshWindow* window, uint8_t frameIndex)
    : mWindow(window),
      mFrameIndex(frameIndex),
      mRenderer(window->mFrames.at(frameIndex).mSkSurface->getCanvas()) {
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

  static std::shared_ptr<SharedResources> Get(IDXGIFactory4* dxgiFactory);
};
std::weak_ptr<Win32Direct3D12GaneshWindow::SharedResources>
  Win32Direct3D12GaneshWindow::gSharedResources;

std::shared_ptr<Win32Direct3D12GaneshWindow::SharedResources>
Win32Direct3D12GaneshWindow::SharedResources::Get(IDXGIFactory4* dxgiFactory) {
  if (auto ret = gSharedResources.lock()) {
    return ret;
  }

  auto ret = std::shared_ptr<SharedResources>(new SharedResources());

#ifndef NDEBUG
  wil::com_ptr<ID3D12Debug> d3d12Debug;
  D3D12GetDebugInterface(IID_PPV_ARGS(d3d12Debug.put()));
  if (d3d12Debug) {
    d3d12Debug->EnableDebugLayer();
  }
#endif

  CheckHResult(dxgiFactory->EnumAdapters1(0, ret->mDXGIAdapter.put()));

  D3D_FEATURE_LEVEL featureLevel {D3D_FEATURE_LEVEL_11_0};
  CheckHResult(D3D12CreateDevice(
    ret->mDXGIAdapter.get(),
    featureLevel,
    IID_PPV_ARGS(ret->mD3DDevice.put())));
  ConfigureD3DDebugLayer(ret->mD3DDevice);

  gSharedResources = ret;
  return ret;
}

void Win32Direct3D12GaneshWindow::InitializeGraphicsAPI() {
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
    D3D12_COMMAND_QUEUE_DESC desc {
      .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
      .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
    };
    CheckHResult(mD3DDevice->CreateCommandQueue(
      &desc, IID_PPV_ARGS(mD3DCommandQueue.put())));
  }

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
  GrD3DBackendContext skiaD3DContext {};
  skiaD3DContext.fAdapter.retain(mDXGIAdapter.get());
  skiaD3DContext.fDevice.retain(mD3DDevice.get());
  skiaD3DContext.fQueue.retain(mD3DCommandQueue.get());
  // skiaD3DContext.fMemoryAllocator can be left as nullptr
  mSkContext = GrDirectContext::MakeDirect3D(skiaD3DContext);
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
  SetRenderAPI(RenderAPI::Skia, std::make_unique<SkiaFontMetricsProvider>());
}

Win32Direct3D12GaneshWindow::~Win32Direct3D12GaneshWindow() {
  this->CleanupFrameContexts();
}

IUnknown* Win32Direct3D12GaneshWindow::GetDirectCompositionTargetDevice()
  const {
  return mD3DCommandQueue.get();
}

void Win32Direct3D12GaneshWindow::CreateRenderTargets() {
  const auto rtvStart = mD3DRTVHeap->GetCPUDescriptorHandleForHeapStart();
  const auto rtvStep = mD3DDevice->GetDescriptorHandleIncrementSize(
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  const auto swapchain = GetSwapChain();
  for (UINT i = 0; i < SwapChainLength; ++i) {
    auto& frame = mFrames[i];
    CheckHResult(
      swapchain->GetBuffer(i, IID_PPV_ARGS(frame.mRenderTarget.put())));
    frame.mRenderTarget->SetName(L"FredEmmott::GUI Skia RenderTarget");
    frame.mRenderTargetView = rtvStart;
    frame.mRenderTargetView.ptr += i * rtvStep;
    mD3DDevice->CreateRenderTargetView(
      frame.mRenderTarget.get(), nullptr, frame.mRenderTargetView);

    DXGI_SWAP_CHAIN_DESC1 desc;
    swapchain->GetDesc1(&desc);

    frame.mRenderTarget->AddRef();
    GrD3DTextureResourceInfo backBufferInfo(
      frame.mRenderTarget.get(),
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
    frame.mSkSurface = SkSurfaces::WrapBackendRenderTarget(
      mSkContext.get(),
      backBufferRT,
      {},
      kRGBA_8888_SkColorType,
      nullptr,
      nullptr);
  }
}

void Win32Direct3D12GaneshWindow::AfterPaintFrame(uint8_t frameIndex) {
  auto& frame = mFrames.at(frameIndex);

  GrD3DFenceInfo fenceInfo {};
  fenceInfo.fFence.retain(mD3DFence.get());
  fenceInfo.fValue = ++mFenceValue;
  frame.mFenceValue = fenceInfo.fValue;
  GrBackendSemaphore flushSemaphore;
  flushSemaphore.initDirect3D(fenceInfo);

  mSkContext->flush(
    frame.mSkSurface.get(),
    SkSurfaces::BackendSurfaceAccess::kPresent,
    GrFlushInfo {
      .fNumSemaphores = 1,
      .fSignalSemaphores = &flushSemaphore,
    });
  mSkContext->submit(GrSyncCpu::kNo);

  CheckHResult(GetSwapChain()->Present(0, 0));
}

void Win32Direct3D12GaneshWindow::CleanupFrameContexts() {
  mSkContext->flushAndSubmit(GrSyncCpu::kYes);

  const auto fenceValue = ++mFenceValue;
  CheckHResult(mD3DCommandQueue->Signal(mD3DFence.get(), fenceValue));
  CheckHResult(mD3DFence->SetEventOnCompletion(fenceValue, mFenceEvent.get()));
  WaitForSingleObject(mFenceEvent.get(), INFINITE);

  for (auto&& frame: mFrames) {
    frame.mSkSurface = {};
    frame.mRenderTarget = nullptr;
    frame.mRenderTargetView = {};
    frame.mFenceValue = {};
  }
}
std::unique_ptr<Win32Window> Win32Direct3D12GaneshWindow::CreatePopup(
  HINSTANCE instance,
  int showCommand,
  const Options& options) const {
  return std::make_unique<Win32Direct3D12GaneshWindow>(
    instance, showCommand, options);
}

std::unique_ptr<Win32Direct3D12GaneshWindow::BasicFramePainter>
Win32Direct3D12GaneshWindow::GetFramePainter(uint8_t mFrameIndex) {
  return std::unique_ptr<BasicFramePainter> {
    new FramePainter(this, mFrameIndex)};
}

void Win32Direct3D12GaneshWindow::BeforePaintFrame(uint8_t frameIndex) {
  const auto& frame = mFrames.at(frameIndex);

  if (!frame.mFenceValue) {
    return;
  }

  mD3DFence->SetEventOnCompletion(frame.mFenceValue, mFenceEvent.get());
  WaitForSingleObject(mFenceEvent.get(), INFINITE);
}

}// namespace FredEmmott::GUI