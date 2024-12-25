// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32-Ganesh-D3D12.hpp"

#include <shlobj_core.h>
#include <skia/core/SkCanvas.h>
#include <skia/core/SkColor.h>
#include <skia/core/SkColorSpace.h>
#include <skia/core/SkImageInfo.h>
#include <skia/gpu/GrBackendSemaphore.h>
#include <skia/gpu/GrBackendSurface.h>
#include <skia/gpu/GrDirectContext.h>
#include <skia/gpu/d3d/GrD3DBackendContext.h>
#include <skia/gpu/ganesh/SkSurfaceGanesh.h>

#include <FredEmmott/GUI/events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp>
#include <FredEmmott/GUI/events/MouseMoveEvent.hpp>
#include <chrono>
#include <filesystem>
#include <format>
#include <print>
#include <source_location>

#include "FredEmmott/GUI/Immediate/Button.hpp"
#include "FredEmmott/GUI/Immediate/Card.hpp"
#include "FredEmmott/GUI/Immediate/Label.hpp"
#include "FredEmmott/GUI/Immediate/Root.hpp"
#include "FredEmmott/GUI/Immediate/StackPanel.hpp"
#include "FredEmmott/GUI/Widgets/ToggleSwitchKnob.hpp"
#include "FredEmmott/GUI/Widgets/ToggleSwitchThumb.hpp"

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

static void PopulateMouseEvent(
  fui::MouseEvent* e,
  WPARAM wParam,
  LPARAM lParam,
  float dpiScale) {
  e->mPoint = {
    LOWORD(lParam) / dpiScale,
    HIWORD(lParam) / dpiScale,
  };

  using fui::MouseButton;
  if (wParam & MK_LBUTTON) {
    e->mButtons |= MouseButton::Left;
  }
  if (wParam & MK_MBUTTON) {
    e->mButtons |= MouseButton::Middle;
  }
  if (wParam & MK_RBUTTON) {
    e->mButtons |= MouseButton::Right;
  }
  if (wParam & MK_XBUTTON1) {
    e->mButtons |= MouseButton::X1;
  }
  if (wParam & MK_XBUTTON2) {
    e->mButtons |= MouseButton::X2;
  }
}

static inline void CheckHResult(
  const HRESULT ret,
  const std::source_location& caller = std::source_location::current()) {
  if (SUCCEEDED(ret)) [[likely]] {
    return;
  }

  const std::error_code ec {ret, std::system_category()};

  const auto msg = std::format(
    "HRESULT failed: {:#010x} @ {} - {}:{}:{} - {}\n",
    std::bit_cast<uint32_t>(ret),
    caller.function_name(),
    caller.file_name(),
    caller.line(),
    caller.column(),
    ec.message());
  OutputDebugStringA(msg.c_str());
  throw std::system_error(ec);
}

template <const GUID& TFolderID>
std::filesystem::path GetKnownFolderPath() {
  wil::unique_cotaskmem_string buf;
  CheckHResult(
    SHGetKnownFolderPath(TFolderID, KF_FLAG_DEFAULT, nullptr, buf.put()));
  std::filesystem::path path {std::wstring_view {buf.get()}};
  if (std::filesystem::exists(path)) {
    return std::filesystem::canonical(path);
  }
  return {};
}

HelloSkiaWindow::HelloSkiaWindow(HINSTANCE instance) {
  gInstance = this;

  this->CreateNativeWindow(instance);
  this->InitializeD3D();
  this->InitializeSkia();
  this->CreateRenderTargets();
}

void HelloSkiaWindow::CreateNativeWindow(HINSTANCE instance) {
  const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
  const auto height = screenHeight / 2;
  const auto width = (height * 2) / 3;

  const WNDCLASSW wc {
    .lpfnWndProc = &WindowProc,
    .hInstance = instance,
    .lpszClassName = L"Hello Skia",
  };
  const auto classAtom = RegisterClassW(&wc);
  mHwnd.reset(CreateWindowExW(
    WS_EX_APPWINDOW | WS_EX_CLIENTEDGE,
    MAKEINTATOM(classAtom),
    L"Hello Skia",
    WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width,
    height,
    nullptr,
    nullptr,
    instance,
    nullptr));

  if (!mHwnd) {
    CheckHResult(HRESULT_FROM_WIN32(GetLastError()));
    return;
  }

  mDPIScale = static_cast<float>(GetDpiForWindow(mHwnd.get()))
    / USER_DEFAULT_SCREEN_DPI;
}

void HelloSkiaWindow::InitializeD3D() {
#ifndef NDEBUG
  wil::com_ptr<ID3D12Debug> d3d12Debug;
  D3D12GetDebugInterface(IID_PPV_ARGS(d3d12Debug.put()));
  if (d3d12Debug) {
    d3d12Debug->EnableDebugLayer();
  }
#endif

  wil::com_ptr<IDXGIFactory4> dxgiFactory;
  {
    UINT flags = 0;
#ifndef NDEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    CheckHResult(CreateDXGIFactory2(flags, IID_PPV_ARGS(dxgiFactory.put())));
  }

  CheckHResult(dxgiFactory->EnumAdapters1(0, mDXGIAdapter.put()));

  D3D_FEATURE_LEVEL featureLevel {D3D_FEATURE_LEVEL_11_0};
  CheckHResult(D3D12CreateDevice(
    mDXGIAdapter.get(), featureLevel, IID_PPV_ARGS(mD3DDevice.put())));
  this->ConfigureD3DDebugLayer();

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

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {1, 0},
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = SwapChainLength,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
  };

  CheckHResult(dxgiFactory->CreateSwapChainForHwnd(
    mD3DCommandQueue.get(),
    mHwnd.get(),
    &swapChainDesc,
    nullptr,
    nullptr,
    mSwapChain.put()));
  CheckHResult(mSwapChain->GetDesc1(&swapChainDesc));
  mWindowSize = {swapChainDesc.Width, swapChainDesc.Height};
}

void HelloSkiaWindow::InitializeSkia() {
  GrD3DBackendContext skiaD3DContext {};
  skiaD3DContext.fAdapter.retain(mDXGIAdapter.get());
  skiaD3DContext.fDevice.retain(mD3DDevice.get());
  skiaD3DContext.fQueue.retain(mD3DCommandQueue.get());
  // skiaD3DContext.fMemoryAllocator can be left as nullptr
  mSkContext = GrDirectContext::MakeDirect3D(skiaD3DContext);
}

void HelloSkiaWindow::ConfigureD3DDebugLayer() {
#ifndef NDEBUG
  auto infoQueue = mD3DDevice.try_query<ID3D12InfoQueue1>();
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

HelloSkiaWindow::~HelloSkiaWindow() {
  this->CleanupFrameContexts();

  gInstance = nullptr;
}

void HelloSkiaWindow::CreateRenderTargets() {
  const auto rtvStart = mD3DRTVHeap->GetCPUDescriptorHandleForHeapStart();
  const auto rtvStep = mD3DDevice->GetDescriptorHandleIncrementSize(
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  for (UINT i = 0; i < SwapChainLength; ++i) {
    auto& frame = mFrames[i];
    CheckHResult(
      mSwapChain->GetBuffer(i, IID_PPV_ARGS(frame.mRenderTarget.put())));
    frame.mRenderTarget->SetName(L"HelloSkia RenderTarget");
    frame.mRenderTargetView = rtvStart;
    frame.mRenderTargetView.ptr += i * rtvStep;
    mD3DDevice->CreateRenderTargetView(
      frame.mRenderTarget.get(), nullptr, frame.mRenderTargetView);

    DXGI_SWAP_CHAIN_DESC1 desc;
    mSwapChain->GetDesc1(&desc);

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

HWND HelloSkiaWindow::GetHWND() const noexcept {
  return mHwnd.get();
}

void HelloSkiaWindow::RenderSkiaContent(SkCanvas* canvas) {
  canvas->resetMatrix();

  canvas->scale(mDPIScale, mDPIScale);
  const auto it = canvas->imageInfo();

  {
    mFUIRoot.BeginFrame();

    fuii::BeginCard();
    fuii::BeginVStackPanel();
    fuii::Label("Hello, world; this text doesn't make the button wider aeg");
    fuii::Label("Frame {}##Frames", mFrameCounter);
    if (fuii::Button("Click Me!")) {
      std::println(stderr, "Clicked!");
    }

    fuii::immediate_detail::BeginWidget<fui::Widgets::ToggleSwitchKnob> {}();
    fuii::immediate_detail::EndWidget<fui::Widgets::ToggleSwitchKnob>();

    fuii::EndStackPanel();
    fuii::EndCard();

    mFUIRoot.EndFrame(
      mWindowSize.mWidth / mDPIScale, mWindowSize.mHeight / mDPIScale, canvas);
  }
}

void HelloSkiaWindow::RenderSkiaContent(FrameContext& frame) {
  try {
    this->RenderSkiaContent(frame.mSkSurface->getCanvas());
  } catch (const std::exception& e) {
    std::println(
      stderr, "Rendering Skia content threw an exception: {}", e.what());
    if (IsDebuggerPresent()) {
      __debugbreak();
    }
    throw;
  }

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
}

void HelloSkiaWindow::RenderFrame() {
  if (mPendingResize) {
    this->CleanupFrameContexts();
    CheckHResult(mSwapChain->ResizeBuffers(
      0,
      mPendingResize->mWidth,
      mPendingResize->mHeight,
      DXGI_FORMAT_UNKNOWN,
      0));
    this->CreateRenderTargets();

    mWindowSize = *mPendingResize;
    mPendingResize = std::nullopt;
  }

  ++mFrameCounter;
  auto& frame = mFrames.at(mFrameIndex);
  mFrameIndex = (mFrameIndex + 1) % SwapChainLength;

  if (frame.mFenceValue) {
    mD3DFence->SetEventOnCompletion(frame.mFenceValue, mFenceEvent.get());
    WaitForSingleObject(mFenceEvent.get(), INFINITE);
  }

  RenderSkiaContent(frame);

  CheckHResult(mSwapChain->Present(0, 0));
}

int HelloSkiaWindow::Run() noexcept {
  std::chrono::milliseconds frameInterval {1000 / MinimumFrameRate};

  while (!mExitCode) {
    const auto frameStart = std::chrono::steady_clock::now();

    MSG msg {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        return mExitCode.value_or(0);
      }
    }

    this->RenderFrame();

    const auto frameDuration = std::chrono::steady_clock::now() - frameStart;
    if (frameDuration > frameInterval) {
      continue;
    }
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                          frameInterval - frameDuration)
                          .count();
    MsgWaitForMultipleObjects(0, nullptr, false, millis, QS_ALLINPUT);
  }

  return *mExitCode;
}

LRESULT HelloSkiaWindow::WindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam) noexcept {
  namespace fui = FredEmmott::GUI;

  switch (uMsg) {
    case WM_SETTINGCHANGE:
      fui::StaticTheme::Refresh();
      break;
    case WM_SIZE: {
      const UINT width = LOWORD(lParam);
      const UINT height = HIWORD(lParam);
      gInstance->mPendingResize = PixelSize {width, height};
      break;
    }
    case WM_DPICHANGED: {
      const auto newDPI = HIWORD(wParam);
      // TODO: lParam is a RECT that we *should* use
      gInstance->mDPIScale
        = static_cast<float>(newDPI) / USER_DEFAULT_SCREEN_DPI;
      break;
    }
    case WM_MOUSEMOVE: {
      fui::MouseMoveEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      e.mChangedButtons = fui::MouseButton::Left;
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      e.mChangedButtons = fui::MouseButton::Left;
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      e.mChangedButtons = fui::MouseButton::Middle;
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      e.mChangedButtons = fui::MouseButton::Middle;
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      e.mChangedButtons = fui::MouseButton::Right;
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      e.mChangedButtons = fui::MouseButton::Right;
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        e.mChangedButtons |= fui::MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        e.mChangedButtons |= fui::MouseButton::X2;
      }
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, gInstance->mDPIScale);
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        e.mChangedButtons |= fui::MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        e.mChangedButtons |= fui::MouseButton::X2;
      }
      gInstance->mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_CLOSE:
      gInstance->mExitCode = 0;
      break;
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void HelloSkiaWindow::CleanupFrameContexts() {
  mSkContext->flushAndSubmit(GrSyncCpu::kYes);

  const auto fenceValue = ++mFenceValue;
  CheckHResult(mD3DCommandQueue->Signal(mD3DFence.get(), fenceValue));
  CheckHResult(mD3DFence->SetEventOnCompletion(fenceValue, mFenceEvent.get()));
  WaitForSingleObject(mFenceEvent.get(), INFINITE);

  for (auto& frame: mFrames) {
    frame.mSkSurface = {};
    frame.mRenderTarget = nullptr;
    frame.mRenderTargetView = {};
    frame.mFenceValue = {};
  }

  mFrameIndex = 0;
}

HelloSkiaWindow* HelloSkiaWindow::gInstance {nullptr};

int WINAPI wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR lpCmdLine,
  int nCmdShow) {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  HelloSkiaWindow app(hInstance);
  ShowWindow(app.GetHWND(), nCmdShow);
  return app.Run();
}
