// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32Direct3D12GaneshWindow.hpp"

#include <dwmapi.h>
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
#include <wil/win32_helpers.h>

#include <FredEmmott/GUI/Immediate/Root.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp>
#include <FredEmmott/GUI/events/MouseMoveEvent.hpp>
#include <chrono>
#include <filesystem>
#include <format>
#include <source_location>

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

namespace {

std::wstring GetDefaultWindowClassName() {
  const auto thisExe = wil::QueryFullProcessImageNameW(GetCurrentProcess(), 0);
  return std::format(
    L"FUI Window - {}",
    std::filesystem::path(thisExe.get()).filename().wstring());
}

void PopulateMouseEvent(
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

void ThrowHResult(
  const HRESULT ret,
  const std::source_location& caller = std::source_location::current()) {
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

void CheckHResult(
  const HRESULT ret,
  const std::source_location& caller = std::source_location::current()) {
  if (SUCCEEDED(ret)) [[likely]] {
    return;
  }
  ThrowHResult(ret, caller);
}

std::wstring Utf8ToWide(std::string_view s) {
  const auto retCharCount = MultiByteToWideChar(
    CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), s.size(), nullptr, 0);
  std::wstring ret;
  ret.resize(retCharCount);
  MultiByteToWideChar(
    CP_UTF8,
    MB_ERR_INVALID_CHARS,
    s.data(),
    s.size(),
    ret.data(),
    retCharCount);
  if (const auto i = ret.find_last_of(L'\0'); i != std::wstring::npos) {
    ret.erase(i);
  }
  return ret;
}

}// namespace

thread_local decltype(Win32Direct3D12GaneshWindow::gInstances)
  Win32Direct3D12GaneshWindow::gInstances {};
thread_local Win32Direct3D12GaneshWindow*
  Win32Direct3D12GaneshWindow::gInstanceCreatingWindow {nullptr};

void Win32Direct3D12GaneshWindow::AdjustToWindowsTheme() {
  BOOL darkMode {
    fui::StaticTheme::GetCurrent() == fui::StaticTheme::Theme::Dark};
  // Support building with the Windows 10 SDK
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
  DwmSetWindowAttribute(
    mHwnd.get(), DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
}
void Win32Direct3D12GaneshWindow::InitializeWindow() {
  this->CreateNativeWindow();
  this->InitializeD3D();
  this->InitializeSkia();
  this->CreateRenderTargets();

  this->AdjustToWindowsTheme();

  ShowWindow(mHwnd.get(), mShowCommand);
}

Win32Direct3D12GaneshWindow::Win32Direct3D12GaneshWindow(
  HINSTANCE hInstance,
  int nCmdShow,
  const Options& options)
  : mInstanceHandle(hInstance), mShowCommand(nCmdShow), mOptions(options) {
}

void Win32Direct3D12GaneshWindow::CreateNativeWindow() {
  const std::wstring className = mOptions.mClass.empty()
    ? GetDefaultWindowClassName()
    : Utf8ToWide(mOptions.mClass);

  const WNDCLASSW wc {
    .lpfnWndProc = &StaticWindowProc,
    .hInstance = mInstanceHandle,
    .lpszClassName = className.c_str(),
  };
  if (!RegisterClassW(&wc)) {
    if (const auto error = GetLastError();
        error != ERROR_CLASS_ALREADY_EXISTS) {
      ThrowHResult(HRESULT_FROM_WIN32(error));
    }
  }

  mWindowStyle = WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX);
  mWindowExStyle = WS_EX_APPWINDOW | WS_EX_CLIENTEDGE;

  gInstanceCreatingWindow = this;
  mMinimumContentSizeInDIPs = mFUIRoot.GetMinimumSize();
  const std::wstring title
    = mOptions.mTitle.empty() ? L"FUI Window" : Utf8ToWide(mOptions.mTitle);
  mHwnd.reset(CreateWindowExW(
    mWindowExStyle,
    className.c_str(),
    title.c_str(),
    mWindowStyle,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    mOptions.mInitialSize.fWidth,
    mOptions.mInitialSize.fHeight,
    nullptr,
    nullptr,
    mInstanceHandle,
    nullptr));
  gInstanceCreatingWindow = nullptr;

  if (!mHwnd) {
    CheckHResult(HRESULT_FROM_WIN32(GetLastError()));
    return;
  }

  gInstances.emplace(mHwnd.get(), this);

  if (!mDPI) {
    mDPI = GetDpiForWindow(mHwnd.get());
    mDPIScale = static_cast<float>(*mDPI) / USER_DEFAULT_SCREEN_DPI;
  }
}

void Win32Direct3D12GaneshWindow::InitializeD3D() {
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
  mWindowSize = {
    static_cast<int32_t>(swapChainDesc.Width),
    static_cast<int32_t>(swapChainDesc.Height),
  };
}

void Win32Direct3D12GaneshWindow::InitializeSkia() {
  GrD3DBackendContext skiaD3DContext {};
  skiaD3DContext.fAdapter.retain(mDXGIAdapter.get());
  skiaD3DContext.fDevice.retain(mD3DDevice.get());
  skiaD3DContext.fQueue.retain(mD3DCommandQueue.get());
  // skiaD3DContext.fMemoryAllocator can be left as nullptr
  mSkContext = GrDirectContext::MakeDirect3D(skiaD3DContext);
}

void Win32Direct3D12GaneshWindow::ConfigureD3DDebugLayer() {
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

Win32Direct3D12GaneshWindow::~Win32Direct3D12GaneshWindow() {
  this->CleanupFrameContexts();

  const auto it = std::ranges::find(
    gInstances, this, [](const auto& pair) { return pair.second; });
  if (it != gInstances.end()) {
    gInstances.erase(it);
  }
}

void Win32Direct3D12GaneshWindow::CreateRenderTargets() {
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

HWND Win32Direct3D12GaneshWindow::GetHWND() const noexcept {
  return mHwnd.get();
}

void Win32Direct3D12GaneshWindow::ResizeIfNeeded() {
  const auto contentMin = mFUIRoot.GetMinimumSize();
  if (contentMin != mMinimumContentSizeInDIPs) {
    mMinimumContentSizeInDIPs = contentMin;
    const auto windowMin = CalculateMinimumWindowSize();
    if (
      mWindowSize.fWidth < windowMin.fWidth
      || mWindowSize.fHeight < windowMin.fHeight) {
      mWindowSize = {
        std::max(mWindowSize.fWidth, windowMin.fWidth),
        std::max(mWindowSize.fHeight, windowMin.fHeight),
      };
      SetWindowPos(
        mHwnd.get(),
        nullptr,
        0,
        0,
        mWindowSize.fWidth,
        mWindowSize.fHeight,
        SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
    }
  }

  if (mPendingResize == mWindowSize) {
    mPendingResize.reset();
    return;
  }

  if (!mPendingResize) {
    return;
  }

  this->CleanupFrameContexts();
  CheckHResult(mSwapChain->ResizeBuffers(
    0,
    mPendingResize->fWidth,
    mPendingResize->fHeight,
    DXGI_FORMAT_UNKNOWN,
    0));
  this->CreateRenderTargets();

  mWindowSize = std::move(*mPendingResize);
  mPendingResize.reset();
}

void Win32Direct3D12GaneshWindow::EndFrame() {
  mFUIRoot.EndFrame();

  if (!mHwnd) [[unlikely]] {
    this->InitializeWindow();
    if (!mHwnd) {
      mExitCode = EXIT_FAILURE;
      return;
    }
  } else {
    this->ResizeIfNeeded();
  }

  auto& frame = mFrames.at(mFrameIndex);
  mFrameIndex = (mFrameIndex + 1) % SwapChainLength;

  if (frame.mFenceValue) {
    mD3DFence->SetEventOnCompletion(frame.mFenceValue, mFenceEvent.get());
    WaitForSingleObject(mFenceEvent.get(), INFINITE);
  }

  SkCanvas* canvas = frame.mSkSurface->getCanvas();
  canvas->resetMatrix();
  canvas->scale(mDPIScale, mDPIScale);

  const SkSize size {
    mWindowSize.fWidth / mDPIScale,
    mWindowSize.fHeight / mDPIScale,
  };

  mFUIRoot.Paint(canvas, size);

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

  CheckHResult(mSwapChain->Present(0, 0));
}

std::expected<void, int> Win32Direct3D12GaneshWindow::BeginFrame() {
  mBeginFrameTime = std::chrono::steady_clock::now();
  MSG msg {};
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (mExitCode.has_value()) {
      return std::unexpected {mExitCode.value_or(0)};
    }
  }
  mFUIRoot.BeginFrame();
  return {};
}

void Win32Direct3D12GaneshWindow::WaitFrame(
  unsigned int minFPS,
  unsigned int maxFPS) const {
  if (minFPS == std::numeric_limits<unsigned int>::max()) {
    return;
  }

  const auto fps = std::clamp<unsigned int>(60, minFPS, maxFPS);
  std::chrono::milliseconds frameInterval {1000 / fps};

  const auto frameDuration = std::chrono::steady_clock::now() - mBeginFrameTime;
  if (frameDuration > frameInterval) {
    return;
  }
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                        frameInterval - frameDuration)
                        .count();
  MsgWaitForMultipleObjects(0, nullptr, false, millis, QS_ALLINPUT);
}

LRESULT Win32Direct3D12GaneshWindow::StaticWindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam) {
  auto it = gInstances.find(hwnd);
  if (it != gInstances.end()) {
    auto& self = *it->second;
    if (hwnd == self.mHwnd.get()) {
      return self.WindowProc(hwnd, uMsg, wParam, lParam);
    }
#ifndef NDEBUG
    // Should *always* match above
    __debugbreak();
#endif
  }
  if (gInstanceCreatingWindow) {
    return gInstanceCreatingWindow->WindowProc(hwnd, uMsg, wParam, lParam);
  }
#ifndef NDEBUG
  __debugbreak();
#endif
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT
Win32Direct3D12GaneshWindow::WindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam) {
  if (mHwnd && hwnd != mHwnd.get()) {
    throw std::logic_error("hwnd mismatch");
  }
  namespace fui = FredEmmott::GUI;
  switch (uMsg) {
    case WM_SETCURSOR: {
      static const wil::unique_hcursor sDefaultCursor {
        LoadCursor(nullptr, IDC_ARROW)};
      const auto hitTest = LOWORD(lParam);
      if (hitTest == HTCLIENT) {
        // Hand: sPointerCursor -> IDC_HAND
        SetCursor(sDefaultCursor.get());
        return true;
      }
      break;
    }
    case WM_SETTINGCHANGE:
      fui::StaticTheme::Refresh();
      this->AdjustToWindowsTheme();
      break;
    case WM_GETMINMAXINFO: {
      if (!mDPI) {
        mDPI = GetDpiForWindow(hwnd);
        mDPIScale = static_cast<float>(*mDPI) / USER_DEFAULT_SCREEN_DPI;
        CalculateMinimumWindowSize();
      }

      if (!mMinimumWindowSize) {
        if (!mMinimumContentSizeInDIPs) {
#ifndef NDEBUG
          __debugbreak();
#endif
          break;
        }
        this->CalculateMinimumWindowSize();
      }
      auto* minInfo = reinterpret_cast<MINMAXINFO*>(lParam);
      minInfo->ptMinTrackSize.x = mMinimumWindowSize->fWidth;
      minInfo->ptMinTrackSize.y = mMinimumWindowSize->fHeight;
      return 0;
    }
    case WM_SIZE: {
      const auto width = LOWORD(lParam);
      const auto height = HIWORD(lParam);
      mPendingResize = {width, height};
      break;
    }
    case WM_DPICHANGED: {
      const auto newDPI = HIWORD(wParam);
      // TODO: lParam is a RECT that we *should* use
      mDPI = newDPI;
      mDPIScale = static_cast<float>(newDPI) / USER_DEFAULT_SCREEN_DPI;
      break;
    }
    case WM_MOUSEMOVE: {
      fui::MouseMoveEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = fui::MouseButton::Left;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = fui::MouseButton::Left;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = fui::MouseButton::Middle;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = fui::MouseButton::Middle;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = fui::MouseButton::Right;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = fui::MouseButton::Right;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONDOWN: {
      fui::MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        e.mChangedButtons |= fui::MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        e.mChangedButtons |= fui::MouseButton::X2;
      }
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONUP: {
      fui::MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        e.mChangedButtons |= fui::MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        e.mChangedButtons |= fui::MouseButton::X2;
      }
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_CLOSE:
      mExitCode = 0;
      break;
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void Win32Direct3D12GaneshWindow::CleanupFrameContexts() {
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
SkISize Win32Direct3D12GaneshWindow::CalculateMinimumWindowSize() {
  if (!mMinimumContentSizeInDIPs) {
    throw std::logic_error(
      "Can't calculate minimum window size without minimum content size");
  }

  RECT rect {
    0,
    0,
    std::lround(mMinimumContentSizeInDIPs->width() * mDPIScale),
    std::lround(mMinimumContentSizeInDIPs->height() * mDPIScale),
  };
  AdjustWindowRectEx(
    &rect, mWindowStyle & ~WS_OVERLAPPED, false, mWindowExStyle);

  mMinimumWindowSize = {
    rect.right - rect.left,
    rect.bottom - rect.top,
  };
  return *mMinimumWindowSize;
}
