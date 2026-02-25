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
        window->mD3DDevice.get(),
        window->mD2DDeviceContext.get(),
        std::make_shared<D3D11CompletionFlag>(
          window->mFence.get(),
          ++window->mFenceValue)) {
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

struct Win32Direct2DWindow::SharedResources {
  wil::com_ptr<ID3D11Device5> mD3DDevice;
  wil::com_ptr<ID3D11DeviceContext> mD3DDeviceContext;
  wil::com_ptr<ID2D1Factory3> mD2DFactory;
  wil::com_ptr<ID2D1Device2> mD2DDevice;
  wil::com_ptr<IDWriteFactory> mDWriteFactory;

  static std::shared_ptr<SharedResources> Get(IDXGIFactory4* dxgiFactory);
};

std::weak_ptr<Win32Direct2DWindow::SharedResources>
  Win32Direct2DWindow::gSharedResources;

std::shared_ptr<Win32Direct2DWindow::SharedResources>
Win32Direct2DWindow::SharedResources::Get(IDXGIFactory4* dxgiFactory) {
  if (auto ret = gSharedResources.lock()) {
    return ret;
  }

  auto ret = std::shared_ptr<SharedResources>(new SharedResources());

  // Create D3D11 device
  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG
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
    ret->mD3DDeviceContext.put()));
  device.query_to(ret->mD3DDevice.put());

  // Create D2D factory
  D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
#ifndef NDEBUG
  d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

  CheckHResult(D2D1CreateFactory(
    D2D1_FACTORY_TYPE_SINGLE_THREADED,
    __uuidof(ID2D1Factory3),
    &d2dFactoryOptions,
    reinterpret_cast<void**>(ret->mD2DFactory.put())));

  // Create D2D device
  const auto dxgiDevice = ret->mD3DDevice.query<IDXGIDevice>();

  CheckHResult(
    ret->mD2DFactory->CreateDevice(dxgiDevice.get(), ret->mD2DDevice.put()));

  // Create DirectWrite factory
  CheckHResult(DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(ret->mDWriteFactory.put())));

  gSharedResources = ret;
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
    std::make_unique<DirectWriteFontProvider>(mDWriteFactory));
}

void Win32Direct2DWindow::InitializeD3D() {
  mSharedResources = SharedResources::Get(this->GetDXGIFactory());
  mD3DDevice = mSharedResources->mD3DDevice;
  mD3DDeviceContext = mSharedResources->mD3DDeviceContext;
  CheckHResult(mD3DDevice->CreateFence(
    mFenceValue, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.put())));
}

void Win32Direct2DWindow::InitializeDirect2D() {
  mD2DFactory = mSharedResources->mD2DFactory;
  mD2DDevice = mSharedResources->mD2DDevice;
  mDWriteFactory = mSharedResources->mDWriteFactory;

  // Create D2D device context
  CheckHResult(mD2DDevice->CreateDeviceContext(
    D2D1_DEVICE_CONTEXT_OPTIONS_NONE, mD2DDeviceContext.put()));
}

Win32Direct2DWindow::Win32Direct2DWindow(
  HINSTANCE instance,
  UINT showCommand,
  const Options& options)
  : Win32Window(instance, showCommand, options) {}

Win32Direct2DWindow::~Win32Direct2DWindow() {
  this->CleanupFrameContexts();
}

IUnknown* Win32Direct2DWindow::GetDirectCompositionTargetDevice() const {
  return mD3DDevice.get();
}

void Win32Direct2DWindow::CreateRenderTargets() {
  const auto swapchain = GetSwapChain();

  // Get the backbuffer from the swapchain
  wil::com_ptr<IDXGISurface> dxgiSurface;
  CheckHResult(swapchain->GetBuffer(0, IID_PPV_ARGS(dxgiSurface.put())));
  CheckHResult(mD2DDeviceContext->CreateBitmapFromDxgiSurface(
    dxgiSurface.get(), nullptr, mFrame.mD2DTargetBitmap.put()));
}

void Win32Direct2DWindow::AfterPaintFrame([[maybe_unused]] uint8_t frameIndex) {
  CheckHResult(mD2DDeviceContext->EndDraw());
  CheckHResult(GetSwapChain()->Present(0, DXGI_PRESENT_ALLOW_TEARING));
  mD2DDeviceContext->SetTarget(nullptr);
}

void Win32Direct2DWindow::CleanupFrameContexts() {
  mD2DDeviceContext->SetTarget(nullptr);
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
  mD2DDeviceContext->SetTarget(mFrame.mD2DTargetBitmap.get());

  mD2DDeviceContext->BeginDraw();
  const auto dpi = GetDPIScale() * USER_DEFAULT_SCREEN_DPI;
  const auto scale = 1 / GetDPIScale();
  mD2DDeviceContext->SetDpi(dpi, dpi);
  mD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale));
}

}// namespace FredEmmott::GUI