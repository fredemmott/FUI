// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32Direct3D12GaneshWindow.hpp"

#include <dwmapi.h>
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
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <FredEmmott/GUI/events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp>
#include <FredEmmott/GUI/events/MouseMoveEvent.hpp>
#include <chrono>
#include <filesystem>
#include <format>
#include <source_location>
#include <thread>

namespace FredEmmott::GUI {
namespace {

std::wstring GetDefaultWindowClassName() {
  const auto thisExe = wil::QueryFullProcessImageNameW(GetCurrentProcess(), 0);
  return std::format(
    L"FUI Window - {}",
    std::filesystem::path(thisExe.get()).filename().wstring());
}

void PopulateMouseEvent(
  MouseEvent* e,
  WPARAM wParam,
  LPARAM lParam,
  float dpiScale) {
  e->mPoint = {
    LOWORD(lParam) / dpiScale,
    HIWORD(lParam) / dpiScale,
  };

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

thread_local decltype(Win32Direct3D12GaneshWindow::gInstances)
  Win32Direct3D12GaneshWindow::gInstances {};
thread_local Win32Direct3D12GaneshWindow*
  Win32Direct3D12GaneshWindow::gInstanceCreatingWindow {nullptr};

struct Win32Direct3D12GaneshWindow::SharedResources {
  wil::com_ptr<IDXGIFactory4> mDXGIFactory;
  wil::com_ptr<IDXGIAdapter1> mDXGIAdapter;
  wil::com_ptr<ID3D12Device> mD3DDevice;
  wil::com_ptr<ID3D12CommandQueue> mD3DCommandQueue;

  static std::shared_ptr<SharedResources> Get();
};
std::weak_ptr<Win32Direct3D12GaneshWindow::SharedResources>
  Win32Direct3D12GaneshWindow::gSharedResources;

std::shared_ptr<Win32Direct3D12GaneshWindow::SharedResources>
Win32Direct3D12GaneshWindow::SharedResources::Get() {
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

  {
    UINT flags = 0;
#ifndef NDEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    CheckHResult(
      CreateDXGIFactory2(flags, IID_PPV_ARGS(ret->mDXGIFactory.put())));
  }

  CheckHResult(ret->mDXGIFactory->EnumAdapters1(0, ret->mDXGIAdapter.put()));

  D3D_FEATURE_LEVEL featureLevel {D3D_FEATURE_LEVEL_11_0};
  CheckHResult(D3D12CreateDevice(
    ret->mDXGIAdapter.get(),
    featureLevel,
    IID_PPV_ARGS(ret->mD3DDevice.put())));
  ConfigureD3DDebugLayer(ret->mD3DDevice);

  gSharedResources = ret;
  return ret;
}

void Win32Direct3D12GaneshWindow::AdjustToWindowsTheme() {
  BOOL darkMode {StaticTheme::GetCurrent() == StaticTheme::Theme::Dark};
  // Support building with the Windows 10 SDK
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
  DwmSetWindowAttribute(
    mHwnd.get(), DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

  DWM_SYSTEMBACKDROP_TYPE backdropType {mOptions.mSystemBackdrop};
  mHaveSystemBackdrop = SUCCEEDED(DwmSetWindowAttribute(
    mHwnd.get(),
    DWMWA_SYSTEMBACKDROP_TYPE,
    &backdropType,
    sizeof(backdropType)));
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
  : mInstanceHandle(hInstance),
    mShowCommand(nCmdShow),
    mOptions(options) {}

void Win32Direct3D12GaneshWindow::CreateNativeWindow() {
  const std::wstring className = mOptions.mClass.empty()
    ? GetDefaultWindowClassName()
    : Utf8ToWide(mOptions.mClass);

  const WNDCLASSW wc {
    .style = CS_HREDRAW | CS_VREDRAW,
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

  gInstanceCreatingWindow = this;
  mMinimumContentSizeInDIPs = mFUIRoot.GetMinimumSize();
  const std::wstring title
    = mOptions.mTitle.empty() ? L"FUI Window" : Utf8ToWide(mOptions.mTitle);
  mHwnd.reset(CreateWindowExW(
    mOptions.mWindowExStyle,
    className.c_str(),
    title.c_str(),
    mOptions.mWindowStyle,
    mOptions.mInitialPosition.fX,
    mOptions.mInitialPosition.fY,
    mOptions.mInitialSize.fWidth,
    mOptions.mInitialSize.fHeight,
    mParentHwnd,
    nullptr,
    mInstanceHandle,
    nullptr));
  gInstanceCreatingWindow = nullptr;
  if (!mHwnd) {
    CheckHResult(HRESULT_FROM_WIN32(GetLastError()));
    return;
  }

  mDPI = GetDpiForWindow(mHwnd.get());
  mDPIScale = static_cast<float>(*mDPI) / USER_DEFAULT_SCREEN_DPI;

  RECT clientRect {};
  GetClientRect(mHwnd.get(), &clientRect);

  if (
    clientRect.right - clientRect.left <= 0
    || clientRect.bottom - clientRect.top <= 0) {
    this->CalculateMinimumWindowSize();
    const auto width = std::max<int>(
      mMinimumWindowSize->fWidth, clientRect.right - clientRect.left);
    const auto height = std::max<int>(
      mMinimumWindowSize->fHeight, clientRect.bottom - clientRect.top);
    SetWindowPos(
      mHwnd.get(),
      nullptr,
      0,
      0,
      width,
      height,
      SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
  }

  gInstances.emplace(mHwnd.get(), this);

  RECT windowRect {};
  GetWindowRect(mHwnd.get(), &windowRect);
  mNCSize = {
    windowRect.right - windowRect.left,
    windowRect.bottom - windowRect.top,
  };
  GetClientRect(mHwnd.get(), &clientRect);
  mClientSize = {
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
  };
}

void Win32Direct3D12GaneshWindow::InitializeD3D() {
  mSharedResources = SharedResources::Get();
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

  CheckHResult(
    DCompositionCreateDevice(nullptr, IID_PPV_ARGS(mCompositionDevice.put())));
  CheckHResult(mCompositionDevice->CreateTargetForHwnd(
    mHwnd.get(), true, mCompositionTarget.put()));
  CheckHResult(mCompositionDevice->CreateVisual(mCompositionVisual.put()));

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
    .Width = static_cast<UINT>(mClientSize.fWidth),
    .Height = static_cast<UINT>(mClientSize.fHeight),
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {1, 0},
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = SwapChainLength,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED,
  };

  CheckHResult(mSharedResources->mDXGIFactory->CreateSwapChainForComposition(
    mD3DCommandQueue.get(), &swapChainDesc, nullptr, mSwapChain.put()));
  mCompositionVisual->SetContent(mSwapChain.get());
  mCompositionTarget->SetRoot(mCompositionVisual.get());
  mCompositionDevice->Commit();
}

void Win32Direct3D12GaneshWindow::InitializeSkia() {
  GrD3DBackendContext skiaD3DContext {};
  skiaD3DContext.fAdapter.retain(mDXGIAdapter.get());
  skiaD3DContext.fDevice.retain(mD3DDevice.get());
  skiaD3DContext.fQueue.retain(mD3DCommandQueue.get());
  // skiaD3DContext.fMemoryAllocator can be left as nullptr
  mSkContext = GrDirectContext::MakeDirect3D(skiaD3DContext);
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

void Win32Direct3D12GaneshWindow::ResizeSwapchain() {
  this->CleanupFrameContexts();
  CheckHResult(mSwapChain->ResizeBuffers(
    0,
    mPendingResize->fWidth,
    mPendingResize->fHeight,
    DXGI_FORMAT_UNKNOWN,
    0));
  this->CreateRenderTargets();
}
void Win32Direct3D12GaneshWindow::ResizeIfNeeded() {
  const auto contentMin = mFUIRoot.GetMinimumSize();
  if (contentMin != mMinimumContentSizeInDIPs) {
    mMinimumContentSizeInDIPs = contentMin;
    const auto windowMin = CalculateMinimumWindowSize();
    if (
      mNCSize.fWidth < windowMin.fWidth
      || mNCSize.fHeight < windowMin.fHeight) {
      mNCSize = {
        std::max(mNCSize.fWidth, windowMin.fWidth),
        std::max(mNCSize.fHeight, windowMin.fHeight),
      };
      SetWindowPos(
        mHwnd.get(),
        nullptr,
        0,
        0,
        mNCSize.fWidth,
        mNCSize.fHeight,
        SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
      RECT clientRect {};
      GetClientRect(mHwnd.get(), &clientRect);
      mClientSize = {
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top,
      };
    }
  }

  if (mPendingResize == SkISize {}) {
    return;
  }

  if (mPendingResize == mNCSize) {
    mPendingResize.reset();
    return;
  }

  if (!mPendingResize) {
    return;
  }

  ResizeSwapchain();

  mNCSize = std::move(*mPendingResize);
  mPendingResize.reset();
}

void Win32Direct3D12GaneshWindow::Paint(const SkISize& realPixelSize) {
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
    realPixelSize.fWidth / mDPIScale,
    realPixelSize.fHeight / mDPIScale,
  };

  canvas->clear(
    mHaveSystemBackdrop ? Color {SK_ColorTRANSPARENT}
                        : Color {StaticTheme::SolidBackgroundFillColorBase});
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
void Win32Direct3D12GaneshWindow::EndFrame() {
  using namespace Immediate::immediate_detail;
  mFUIRoot.EndFrame();

  FUI_ASSERT(tWindow == this, "Improperly nested windows");
  tWindow = nullptr;

  if (!mHwnd) [[unlikely]] {
    this->InitializeWindow();
    if (!mHwnd) {
      mExitCode = EXIT_FAILURE;
      return;
    }
  } else {
    this->ResizeIfNeeded();
  }

  this->Paint(mClientSize);
}

void Win32Direct3D12GaneshWindow::SetParent(HWND value) {
  if (mParentHwnd == value) {
    return;
  }
  mParentHwnd = value;
  FUI_ASSERT(!(value && mHwnd), "Parent must be set before window is created");
}

void Win32Direct3D12GaneshWindow::SetInitialPosition(const SkIPoint& topLeft) {
  FUI_ASSERT(!mHwnd, "Initial position must be set before window is created");
  mOptions.mInitialPosition = topLeft;
}

SkIPoint Win32Direct3D12GaneshWindow::CanvasPointToNativePoint(
  const SkIPoint& point) {
  FUI_ASSERT(mDPI && mHwnd);

  SkIPoint ret = point;
  ret.fX *= mDPIScale;
  ret.fY *= mDPIScale;

  // Adjust an all-zero rect to get padding
  RECT rect {};
  AdjustWindowRectEx(
    &rect, mOptions.mWindowStyle, false, mOptions.mWindowExStyle);
  // Top and left padding will be <= 0
  ret.fX -= rect.left;
  ret.fY -= rect.top;

  GetWindowRect(mHwnd.get(), &rect);
  ret.fX += rect.left;
  ret.fY += rect.top;

  return ret;
}

std::expected<void, int> Win32Direct3D12GaneshWindow::BeginFrame() {
  using namespace Immediate::immediate_detail;
  FUI_ASSERT(!tWindow);
  tWindow = this;

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

  const auto fps = std::clamp<unsigned int>(
    mFUIRoot.GetFrameRateRequirement() == FrameRateRequirement::SmoothAnimation
      ? 60
      : 0,
    minFPS,
    maxFPS);
  if (fps == 0) {
    MsgWaitForMultipleObjects(0, nullptr, false, INFINITE, QS_ALLINPUT);
  }
  std::chrono::milliseconds frameInterval {1000 / maxFPS};

  const auto frameDuration = std::chrono::steady_clock::now() - mBeginFrameTime;
  if (frameDuration >= frameInterval) {
    return;
  }
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
    frameInterval - frameDuration);

  std::this_thread::sleep_for(millis);
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
      StaticTheme::Refresh();
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
      if (width > 0 && height > 0) {
        mPendingResize = {width, height};
      }
      break;
    }
    case WM_PAINT:
      if (mPendingResize) {
        this->ResizeSwapchain();
        this->Paint(*mPendingResize);
      }
      break;
    case WM_SIZING: {
      // Initially this is the full window size, including the non-client area
      RECT rect = *reinterpret_cast<RECT*>(lParam);
      // Let's figure out how the client relates, and adjust from there
      RECT padding {};
      AdjustWindowRectEx(
        &padding, mOptions.mWindowStyle, false, mOptions.mWindowExStyle);
      padding.left = -padding.left;
      padding.top = -padding.top;
      mPendingResize = SkISize {
        (rect.right - padding.right) - (rect.left + padding.left),
        (rect.bottom - padding.bottom) - (rect.top + padding.top),
      };
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
      MouseMoveEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONDOWN: {
      MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = MouseButton::Left;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONUP: {
      MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = MouseButton::Left;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONDOWN: {
      MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = MouseButton::Middle;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONUP: {
      MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = MouseButton::Middle;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONDOWN: {
      MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = MouseButton::Right;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONUP: {
      MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      e.mChangedButtons = MouseButton::Right;
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONDOWN: {
      MouseButtonPressEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        e.mChangedButtons |= MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        e.mChangedButtons |= MouseButton::X2;
      }
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONUP: {
      MouseButtonReleaseEvent e;
      PopulateMouseEvent(&e, wParam, lParam, mDPIScale);
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        e.mChangedButtons |= MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        e.mChangedButtons |= MouseButton::X2;
      }
      mFUIRoot.DispatchEvent(&e);
      break;
    }
    case WM_KILLFOCUS:
      if ((mOptions.mWindowStyle & WS_POPUP) == WS_POPUP) {
        mExitCode = 0;
      }
      break;
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
    &rect,
    mOptions.mWindowStyle & ~WS_OVERLAPPED,
    false,
    mOptions.mWindowExStyle);

  mMinimumWindowSize = {
    rect.right - rect.left,
    rect.bottom - rect.top,
  };
  return *mMinimumWindowSize;
}

}// namespace FredEmmott::GUI