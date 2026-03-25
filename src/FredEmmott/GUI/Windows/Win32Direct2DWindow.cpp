// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32Direct2DWindow.hpp"

#include <Windowsx.h>
#include <dwmapi.h>
#include <wil/win32_helpers.h>

#include <FredEmmott/GUI/Direct2DRenderer.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/detail/direct_write_detail/DirectWriteFontProvider.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>
#include <felly/numeric_cast.hpp>
#include <format>

namespace FredEmmott::GUI {

using namespace win32_detail;
using namespace direct_write_detail;

namespace {
struct D3D11CompletionFlag : GPUCompletionFlag {
  D3D11CompletionFlag(ID3D11Fence* const fence, const uint64_t fenceValue)
    : mEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
      mFence(fence),
      mFenceValue(fenceValue) {
    CheckHResult(mFence->SetEventOnCompletion(fenceValue, mEvent.get()));
  }
  ~D3D11CompletionFlag() override = default;

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
  wil::com_ptr<ID3D11Fence> mFence {nullptr};
  uint64_t mFenceValue {};
};

}// namespace

class Win32Direct2DWindow::FramePainter final : public BasicFramePainter {
 public:
  FramePainter() = delete;
  FramePainter(Win32Direct2DWindow* window, uint8_t frameIndex)
    : mWindow(window),
      mFrameIndex(frameIndex),
      mRenderer(
        felly::numeric_cast<uint64_t>(
          std::lround(window->GetDPIScale() * USER_DEFAULT_SCREEN_DPI)),
        window->mRendererDeviceResources,
        std::make_shared<D3D11CompletionFlag>(
          window->mFence.get(),
          window->mFrame.mFenceValue = ++window->mUsedFenceValue)) {
    mWindow->BeforePaintFrame(frameIndex);
  }

  ~FramePainter() override {
    mWindow->AfterPaintFrame(mFrameIndex);
  }

  Renderer* GetRenderer() noexcept override {
    return &mRenderer;
  }

 private:
  Win32Direct2DWindow* mWindow {nullptr};
  uint8_t mFrameIndex {};
  Direct2DRenderer mRenderer;
};

std::shared_ptr<Win32Direct2DWindow::DeviceResources>
Win32Direct2DWindow::GetSharedResources(IDXGIFactory4* dxgiFactory) {
  thread_local std::weak_ptr<DeviceResources> sSharedResources;
  if (const auto ret = sSharedResources.lock()) {
    return ret;
  }

  auto ret = std::make_shared<DeviceResources>();
  sSharedResources = ret;

  // Create D3D11 device
  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef FUI_DEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
  };

  wil::com_ptr<IDXGIAdapter> adapter;
  CheckHResult(dxgiFactory->EnumAdapters(0, adapter.put()));

  D3D_FEATURE_LEVEL featureLevel;
  wil::com_ptr<ID3D11Device> device;
  wil::com_ptr<ID3D11DeviceContext> deviceContext;
  CheckHResult(D3D11CreateDevice(
    adapter.get(),
    D3D_DRIVER_TYPE_UNKNOWN,
    nullptr,
    creationFlags,
    featureLevels,
    ARRAYSIZE(featureLevels),
    D3D11_SDK_VERSION,
    device.put(),
    &featureLevel,
    deviceContext.put()));
  device.query_to(ret->mD3DDevice.put());
  deviceContext.query_to(ret->mD3DDeviceContext.put());

  // Create D2D factory
  D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
#ifdef FUI_DEBUG
  d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

  CheckHResult(D2D1CreateFactory(
    D2D1_FACTORY_TYPE_SINGLE_THREADED,
    __uuidof(ID2D1Factory3),
    &d2dFactoryOptions,
    reinterpret_cast<void**>(ret->mD2DFactory.put())));

  const auto dxgiDevice = ret->mD3DDevice.query<IDXGIDevice>();
  CheckHResult(
    ret->mD2DFactory->CreateDevice(dxgiDevice.get(), ret->mD2DDevice.put()));
  CheckHResult(DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(ret->mDWriteFactory.put())));

  CheckHResult(ret->mD2DFactory->CreateStrokeStyle(
    D2D1::StrokeStyleProperties(
      D2D1_CAP_STYLE_ROUND,
      D2D1_CAP_STYLE_ROUND,
      D2D1_CAP_STYLE_ROUND,
      D2D1_LINE_JOIN_ROUND),
    nullptr,
    0,
    ret->mD2DStrokeStyleRoundCap.put()));

  CheckHResult(ret->mD2DFactory->CreateStrokeStyle(
    D2D1::StrokeStyleProperties(
      D2D1_CAP_STYLE_SQUARE, D2D1_CAP_STYLE_SQUARE, D2D1_CAP_STYLE_SQUARE),
    nullptr,
    0,
    ret->mD2DStrokeStyleSquareCap.put()));

  return ret;
}

void Win32Direct2DWindow::InitializeGraphicsAPI() {
  this->InitializeD3D();
  this->InitializeDirect2D();

  using namespace renderer_detail;
  if (HaveRenderAPI(RenderAPI::Direct2D)) {
    return;
  }

  SetRenderAPI(
    RenderAPI::Direct2D,
    "D2D+DWrite+D3D11",
    std::make_unique<DirectWriteFontProvider>(mDeviceResources.mDWriteFactory));
}

void Win32Direct2DWindow::InitializeD3D() {
  mSharedResources = GetSharedResources(this->GetDXGIFactory());
  mDeviceResources = *mSharedResources;
  CheckHResult(mDeviceResources.mD3DDevice->CreateFence(
    0, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.put())));
}

void Win32Direct2DWindow::InitializeDirect2D() {
  FUI_ASSERT(
    !mDeviceResources.mD2DDeviceContext,
    "D2D context should not be shared between windows");

  // Create D2D device context
  CheckHResult(mDeviceResources.mD2DDevice->CreateDeviceContext(
    D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
    mDeviceResources.mD2DDeviceContext.put()));
  const auto& dr = mDeviceResources;
  mRendererDeviceResources = {
    dr.mD3DDevice.get(),
    dr.mD3DDeviceContext.get(),
    dr.mD2DFactory.get(),
    dr.mD2DDeviceContext.get(),
    dr.mD2DStrokeStyleRoundCap.get(),
    dr.mD2DStrokeStyleSquareCap.get(),
  };
}

Win32Direct2DWindow::Win32Direct2DWindow(
  HINSTANCE instance,
  UINT showCommand,
  const Options& options)
  : Win32Window(instance, showCommand, options) {}

Win32Direct2DWindow::~Win32Direct2DWindow() {
  this->DestroyWindow();
}

IUnknown* Win32Direct2DWindow::GetDirectCompositionTargetDevice() const {
  return mDeviceResources.mD3DDevice.get();
}

void Win32Direct2DWindow::CreateRenderTargets() {
  const auto swapchain = GetSwapChain();

  // Get the backbuffer from the swapchain
  wil::com_ptr<IDXGISurface> dxgiSurface;
  CheckHResult(swapchain->GetBuffer(0, IID_PPV_ARGS(dxgiSurface.put())));
  CheckHResult(mDeviceResources.mD2DDeviceContext->CreateBitmapFromDxgiSurface(
    dxgiSurface.get(), nullptr, mFrame.mD2DTargetBitmap.put()));
}

void Win32Direct2DWindow::AfterPaintFrame([[maybe_unused]] uint8_t frameIndex) {
  const auto d2d = mDeviceResources.mD2DDeviceContext.get();
  const auto d3d = mDeviceResources.mD3DDeviceContext.get();
  CheckHResult(d2d->EndDraw());
  CheckHResult(GetSwapChain()->Present(0, DXGI_PRESENT_ALLOW_TEARING));
  CheckHResult(d3d->Signal(mFence.get(), mFrame.mFenceValue));
  d2d->SetTarget(nullptr);
}

void Win32Direct2DWindow::CleanupFrameContexts() {
  mDeviceResources.mD2DDeviceContext->SetTarget(nullptr);
  mFrame = {};
}

std::unique_ptr<Win32Window> Win32Direct2DWindow::CreatePopup(
  HINSTANCE instance,
  int showCommand,
  const Options& options) const {
  return std::make_unique<Win32Direct2DWindow>(instance, showCommand, options);
}

std::unique_ptr<Win32Direct2DWindow::BasicFramePainter>
Win32Direct2DWindow::GetFramePainter(uint8_t frameIndex) {
  return std::unique_ptr<BasicFramePainter> {
    new FramePainter(this, frameIndex)};
}

void Win32Direct2DWindow::BeforePaintFrame(
  [[maybe_unused]] uint8_t frameIndex) {
  const auto ctx = mDeviceResources.mD2DDeviceContext.get();
  ctx->SetTarget(mFrame.mD2DTargetBitmap.get());
  ctx->BeginDraw();
  const auto dpi = GetDPIScale() * USER_DEFAULT_SCREEN_DPI;
  const auto scale = 1 / GetDPIScale();
  ctx->SetDpi(dpi, dpi);
  ctx->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale));
}

}// namespace FredEmmott::GUI