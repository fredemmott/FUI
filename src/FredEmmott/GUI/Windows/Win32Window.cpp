// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32Window.hpp"

#include <Windows.h>
#include <Windowsx.h>
#include <roapi.h>
#include <wil/resource.h>
#include <wil/win32_helpers.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>
#include <filesystem>
#include <print>

#include "FredEmmott/GUI/ExitException.hpp"

#ifdef FUI_ENABLE_SKIA
#include <FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.hpp>
#endif
#ifdef FUI_ENABLE_DIRECT2D
#include <FredEmmott/GUI/Windows/Win32Direct2DWindow.hpp>
#endif

namespace FredEmmott::GUI {
using namespace win32_detail;

namespace {
std::wstring GetDefaultWindowClassName() {
  const auto thisExe = wil::QueryFullProcessImageNameW(GetCurrentProcess(), 0);
  return std::format(
    L"FredEmmott::GUI (FUI) Window - {}",
    std::filesystem::path(thisExe.get()).filename().wstring());
}

MouseEvent MakeMouseEvent(WPARAM wParam, LPARAM lParam, float dpiScale) {
  MouseEvent ret;
  ret.mWindowPoint = {
    GET_X_LPARAM(lParam) / dpiScale,
    GET_Y_LPARAM(lParam) / dpiScale,
  };

  if (wParam & MK_LBUTTON) {
    ret.mButtons |= MouseButton::Left;
  }
  if (wParam & MK_MBUTTON) {
    ret.mButtons |= MouseButton::Middle;
  }
  if (wParam & MK_RBUTTON) {
    ret.mButtons |= MouseButton::Right;
  }
  if (wParam & MK_XBUTTON1) {
    ret.mButtons |= MouseButton::X1;
  }
  if (wParam & MK_XBUTTON2) {
    ret.mButtons |= MouseButton::X2;
  }

  return ret;
}

}// namespace

thread_local decltype(Win32Window::gInstances) Win32Window::gInstances {};
thread_local Win32Window* Win32Window::gInstanceCreatingWindow {nullptr};

void Win32Window::AdjustToWindowsTheme() {
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

void Win32Window::InitializeWindow() {
  this->CreateNativeWindow();
  this->InitializeGraphicsAPI();
  this->InitializeDirectComposition();
  this->CreateRenderTargets();

  this->AdjustToWindowsTheme();

  ShowWindow(mHwnd.get(), mShowCommand);
}

void Win32Window::HideWindow() {
  if (!mHwnd) {
    return;
  }
  ShowWindow(mHwnd.get(), SW_HIDE);

  if (!mParentHwnd) {
    return;
  }
  EnableWindow(mParentHwnd, TRUE);
  SetForegroundWindow(mParentHwnd);
}

Win32Window::Win32Window(
  const HINSTANCE hInstance,
  int nCmdShow,
  const Options& options)
  : Window(SwapChainLength),
    mInstanceHandle(hInstance),
    mShowCommand(nCmdShow),
    mOptions(options) {
  if (options.mDXGIFactory) {
    mDXGIFactory = wil::com_query<IDXGIFactory4>(options.mDXGIFactory);
  } else {
    UINT flags = 0;
#ifndef NDEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    CheckHResult(CreateDXGIFactory2(flags, IID_PPV_ARGS(mDXGIFactory.put())));
    mOptions.mDXGIFactory = mDXGIFactory.get();
  }
}

unique_ptr<Win32Window> Win32Window::CreateAny(
  HINSTANCE hinstance,
  int showCommand,
  const Options& options) {
#if defined(FUI_ENABLE_DIRECT2D)
  return std::make_unique<Win32Direct2DWindow>(hinstance, showCommand, options);
#elif defined(FUI_ENABLE_SKIA)
  return std::make_unique<Win32Direct3D12GaneshWindow>(
    hinstance, showCommand, options);
#endif
  return nullptr;
}

int Win32Window::WinMain(
  HINSTANCE hInstance,
  [[maybe_unused]] HINSTANCE hPrevInstance,
  [[maybe_unused]] LPWSTR lpCmdLine,
  int nCmdShow,
  void (*appTick)(Win32Window&),
  const WindowOptions& windowOptions,
  const WinMainOptions& options) {
  void (*comCleanupFun)() {nullptr};
  const auto cleanupCOM = wil::scope_exit([options, &comCleanupFun]() {
    if (
      comCleanupFun
      && options.mCOMCleanupMode
        == WinMainOptions::COMCleanupMode::Uninitialize) {
      comCleanupFun();
    }
  });

  using COMMode = WinMainOptions::COMMode;
  switch (options.mCOMMode) {
    case COMMode::Uninitialized:
      break;
    case COMMode::WinRTMultithreaded:
      CheckHResult(RoInitialize(RO_INIT_MULTITHREADED));
      comCleanupFun = &RoUninitialize;
      break;
  }

  using DPIMode = WinMainOptions::DPIMode;
  switch (options.mDPIMode) {
    case DPIMode::Uninitialized:
      break;
    case DPIMode::PerMonitorV2:
      SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
      break;
  }

  if (options.mHooks.mBeforeWindow) {
    options.mHooks.mBeforeWindow();
  }

  unique_ptr<Win32Window> window;
  if (options.mHooks.mCreateWindow) {
    window = options.mHooks.mCreateWindow(hInstance, nCmdShow, windowOptions);
  } else {
    window = CreateAny(hInstance, nCmdShow, windowOptions);
  }

  if (options.mHooks.mBeforeMainLoop) {
    options.mHooks.mBeforeMainLoop(*window);
  }

  while (true) {
    // Variable FPS - wait for whichever is sooner:
    // - input
    // - target frame interval
    //
    // The default target FPS varies; it is '0 fps - input only' usually, but
    // is 60FPS when an animation is active.
    window->WaitFrame();

    if (const auto ok = window->BeginFrame(); !ok) {
      if (options.mHooks.mAfterMainLoop) {
        options.mHooks.mAfterMainLoop(*window, ok.error());
      }
      return ok.error();
    }

    try {
      appTick(*window);
    } catch (const ExitException& e) {
      return e.GetExitCode();
    }

    // Check this before EndFrame() so that users don't have to call EndWidget()
    // correctly if they're quitting anyway
    if (const auto ec = window->GetExitCode()) {
      return *ec;
    }

    window->EndFrame();
  }
}

void Win32Window::TrackMouseEvent() {
  if (mTrackingMouseEvents) {
    return;
  }
  TRACKMOUSEEVENT tme {
    .cbSize = sizeof(tme),
    .dwFlags = TME_LEAVE,
    .hwndTrack = mHwnd.get(),
    .dwHoverTime = HOVER_DEFAULT,
  };
  ::TrackMouseEvent(&tme);
  mTrackingMouseEvents = true;
}

void Win32Window::CreateNativeWindow() {
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
  const std::wstring title
    = mOptions.mTitle.empty() ? L"FUI Window" : Utf8ToWide(mOptions.mTitle);
  mHwnd.reset(CreateWindowExW(
    mOptions.mWindowExStyle,
    className.c_str(),
    title.c_str(),
    mOptions.mWindowStyle,
    mOptions.mInitialPosition.mX,
    mOptions.mInitialPosition.mY,
    0,
    0,
    mParentHwnd,
    nullptr,
    mInstanceHandle,
    nullptr));
  gInstanceCreatingWindow = nullptr;
  if (!mHwnd) {
    CheckHResult(HRESULT_FROM_WIN32(GetLastError()));
    return;
  }

  if (mParentHwnd) {
    gInstances.at(mParentHwnd)->mChildren.push_back(mHwnd.get());
  }

  this->TrackMouseEvent();

  this->SetDPI(GetDpiForWindow(mHwnd.get()));

  gInstances.emplace(mHwnd.get(), this);

  GetWindowRect(mHwnd.get(), &mNCRect);
  {
    const auto [cx, cy] = this->GetInitialWindowSize();
    mNCRect.right = mNCRect.left + cx;
    mNCRect.bottom = mNCRect.top + cy;
  }
  if (mOffsetToChild) {
    YGNodeCalculateLayout(
      this->GetRoot()->GetLayoutNode(),
      mNCRect.right - mNCRect.left,
      YGUndefined,
      YGDirectionLTR);
    const auto canvas = mOffsetToChild->GetTopLeftCanvasPoint();
    const auto native = CanvasPointToNativePoint(canvas);
    const auto nativeOrigin = CanvasPointToNativePoint({0, 0});
    FUI_ASSERT(nativeOrigin.mX != CW_USEDEFAULT);
    FUI_ASSERT(nativeOrigin.mY != CW_USEDEFAULT);

    const auto dx = native.mX - nativeOrigin.mX;
    const auto dy = native.mY - nativeOrigin.mY;
    mNCRect.left -= dx;
    mNCRect.right -= dx;
    mNCRect.top -= dy;
    mNCRect.bottom -= dy;
  }
  this->ApplySizeConstraints(&mNCRect);

  SetWindowPos(
    mHwnd.get(),
    nullptr,
    mNCRect.left,
    mNCRect.top,
    (mNCRect.right - mNCRect.left),
    (mNCRect.bottom - mNCRect.top),
    SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOZORDER);
  GetWindowRect(mHwnd.get(), &mNCRect);
  RECT clientRect {};
  GetClientRect(mHwnd.get(), &clientRect);
  mClientSize = {
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
  };
}

Win32Window::~Win32Window() {
  this->HideWindow();
  if (mParentHwnd && gInstances.contains(mParentHwnd)) {
    auto& siblings = gInstances.at(mParentHwnd)->mChildren;
    siblings.erase(std::ranges::find(siblings, mHwnd.get()));
  }

  const auto it = std::ranges::find(
    gInstances, this, [](const auto& pair) { return pair.second; });
  if (it != gInstances.end()) {
    gInstances.erase(it);
  }
}

void Win32Window::SetSystemBackdropType(const DWM_SYSTEMBACKDROP_TYPE type) {
  if (mOptions.mSystemBackdrop == type) {
    return;
  }
  mOptions.mSystemBackdrop = type;

  if (!mHwnd) {
    return;
  }
  mHaveSystemBackdrop = SUCCEEDED(DwmSetWindowAttribute(
    mHwnd.get(),
    DWMWA_SYSTEMBACKDROP_TYPE,
    &mOptions.mSystemBackdrop,
    sizeof(mOptions.mSystemBackdrop)));
}

void Win32Window::InitializeDirectComposition() {
  CheckHResult(
    DCompositionCreateDevice(nullptr, IID_PPV_ARGS(mCompositionDevice.put())));
  CheckHResult(mCompositionDevice->CreateTargetForHwnd(
    mHwnd.get(), true, mCompositionTarget.put()));
  CheckHResult(mCompositionDevice->CreateVisual(mCompositionVisual.put()));

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
    .Width = static_cast<UINT>(mClientSize.cx),
    .Height = static_cast<UINT>(mClientSize.cy),
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {1, 0},
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = SwapChainLength,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED,
    .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
  };

  CheckHResult(mDXGIFactory->CreateSwapChainForComposition(
    this->GetDirectCompositionTargetDevice(),
    &swapChainDesc,
    nullptr,
    mSwapChain.put()));
  CheckHResult(mCompositionVisual->SetContent(mSwapChain.get()));
  CheckHResult(mCompositionTarget->SetRoot(mCompositionVisual.get()));
  CheckHResult(mCompositionDevice->Commit());
}

void Win32Window::ResizeSwapchain() {
  this->CleanupFrameContexts();
  CheckHResult(mSwapChain->ResizeBuffers(
    0,
    mClientSize.cx,
    mClientSize.cy,
    DXGI_FORMAT_UNKNOWN,
    DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));
  this->CreateRenderTargets();
  this->ResetToFirstBackBuffer();
}

void Win32Window::ResizeIfNeeded() {
  if (!std::exchange(mPendingResize, false)) {
    return;
  }

  RECT clientRect {};
  GetClientRect(mHwnd.get(), &clientRect);
  SIZE clientSize = {
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
  };
  mClientSize = clientSize;

  GetWindowRect(mHwnd.get(), &mNCRect);

  ResizeSwapchain();
}

Size Win32Window::GetClientAreaSize() const {
  return {
    std::floor(static_cast<float>(mClientSize.cx) / mDPIScale),
    std::floor(static_cast<float>(mClientSize.cy) / mDPIScale),
  };
}

float Win32Window::GetDPIScale() const {
  return mDPIScale;
}

Color Win32Window::GetClearColor() const {
  return mHaveSystemBackdrop
    ? Colors::Transparent
    : Color {StaticTheme::Common::SolidBackgroundFillColorBase};
}

void Win32Window::OffsetPositionToDescendant(Widgets::Widget* child) {
  FUI_ASSERT(child);
  if (mHwnd) {
    return;
  }
  FUI_ASSERT(
    mOptions.mInitialPosition.mX != CW_USEDEFAULT,
    "Can't align a child element if an initial position is not specified");
  FUI_ASSERT(
    mOptions.mInitialPosition.mY != CW_USEDEFAULT,
    "Can't align a child element if an initial position is not specified");
  mOffsetToChild = child;
}

void Win32Window::ApplySizeConstraints() {
  auto rect = mNCRect;
  this->ApplySizeConstraints(&rect);
  if (memcmp(&rect, &mNCRect, sizeof(rect)) == 0) {
    return;
  }

  SetWindowPos(
    mHwnd.get(),
    nullptr,
    rect.left,
    rect.top,
    rect.right - rect.left,
    rect.bottom - rect.top,
    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
  GetWindowRect(mHwnd.get(), &mNCRect);
  GetClientRect(mHwnd.get(), &rect);
  mClientSize = {rect.right - rect.left, rect.bottom - rect.top};
  if (mSwapChain) {
    this->ResizeSwapchain();
  }
}

void Win32Window::ApplySizeConstraints(RECT* ncrect) const {
  const_cast<Win32Window*>(this)->WMSizingProc(
    WMSZ_BOTTOMRIGHT, reinterpret_cast<LPARAM>(ncrect));

  const auto monitor = MonitorFromWindow(mHwnd.get(), MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo {sizeof(monitorInfo)};
  GetMonitorInfoW(monitor, &monitorInfo);
  if (ncrect->right > monitorInfo.rcWork.right) {
    const auto dx = ncrect->right - monitorInfo.rcWork.right;
    ncrect->left -= dx;
    ncrect->right -= dx;
  }
  if (ncrect->bottom > monitorInfo.rcWork.bottom) {
    const auto dy = ncrect->bottom - monitorInfo.rcWork.bottom;
    ncrect->bottom -= dy;
    ncrect->top -= dy;
  }
}
void Win32Window::ResizeToIdeal() {
  auto rect = mNCRect;
  const auto size = GetInitialWindowSize();
  rect.right = rect.left + size.cx;
  rect.bottom = rect.top + size.cy;
  WMSizingProc(WMSZ_BOTTOMRIGHT, reinterpret_cast<LPARAM>(&rect));
  SetWindowPos(
    mHwnd.get(),
    nullptr,
    rect.left,
    rect.top,
    (rect.right - rect.left),
    (rect.bottom - rect.top),
    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
  GetWindowRect(mHwnd.get(), &mNCRect);
  GetClientRect(mHwnd.get(), &rect);
  mClientSize = {rect.right - rect.left, rect.bottom - rect.top};
}

void Win32Window::SetParent(NativeHandle value) {
  if (mParentHwnd == value) {
    return;
  }
  mParentHwnd = value;
  FUI_ASSERT(!(value && mHwnd), "Parent must be set before window is created");
}

void Win32Window::SetInitialPositionInNativeCoords(const NativePoint& native) {
  FUI_ASSERT(!mHwnd, "Initial position must be set before window is created");
  mOptions.mInitialPosition = native;
}

NativePoint Win32Window::CanvasPointToNativePoint(const Point& canvas) const {
  FUI_ASSERT(mDPI && mHwnd);

  NativePoint native {
    static_cast<int>(std::round(canvas.mX * mDPIScale)),
    static_cast<int>(std::round(canvas.mY * mDPIScale)),
  };

  // Adjust an all-zero rect to get padding
  RECT rect {};
  AdjustWindowRectEx(
    &rect, mOptions.mWindowStyle, false, mOptions.mWindowExStyle);
  // The top and left padding will both be <= 0
  native.mX -= rect.left;
  native.mY -= rect.top;

  GetWindowRect(mHwnd.get(), &rect);
  native.mX += rect.left;
  native.mY += rect.top;

  return native;
}

LRESULT Win32Window::StaticWindowProc(
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

void Win32Window::SetDPI(const WORD newDPI) {
  mDPI = newDPI;
  mDPIScale = static_cast<float>(newDPI) / USER_DEFAULT_SCREEN_DPI;
  YGConfigSetPointScaleFactor(GetYogaConfig(), mDPIScale);
}

std::optional<LRESULT> Win32Window::WMSizingProc(WPARAM wParam, LPARAM lParam) {
  if (!mDPI) {
    return {};
  }

  if (
    mHorizontalResizeMode == ResizeMode::Allow
    && mVerticalResizeMode == ResizeMode::Allow) {
    return {};
  }

  RECT& rect = *reinterpret_cast<RECT*>(lParam);

  const auto monitor = MonitorFromWindow(mHwnd.get(), MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo {sizeof(monitorInfo)};
  GetMonitorInfoW(monitor, &monitorInfo);

  RECT ncPadding {};
  AdjustWindowRectExForDpi(
    &ncPadding,
    mOptions.mWindowStyle & ~WS_OVERLAPPED,
    false,
    mOptions.mWindowExStyle,
    mDPI.value());
  // Make all values positive
  ncPadding.left = -ncPadding.left;
  ncPadding.top = -ncPadding.top;

  const auto initialWidth = std::lround(
    std::ceil(
      std::ceil(GetMinimumWidth(this->GetRoot()->GetLayoutNode()))
      * mDPIScale));
  const auto requestedWidth
    = rect.right - (rect.left + ncPadding.left + ncPadding.right);
  const auto monitorWidth = monitorInfo.rcWork.right
    - (monitorInfo.rcWork.left + ncPadding.left + ncPadding.right);
  auto targetWidth = requestedWidth;

  switch (mHorizontalResizeMode) {
    case ResizeMode::Fixed:
      targetWidth = initialWidth;
      break;
    case ResizeMode::AllowGrow:
      targetWidth = std::max(requestedWidth, initialWidth);
      break;
    case ResizeMode::AllowShrink:
      targetWidth = std::min(requestedWidth, initialWidth);
      targetWidth = std::min(targetWidth, monitorWidth);
      break;
    case ResizeMode::Allow:
      targetWidth = std::min(requestedWidth, monitorWidth);
      break;
  }
  const auto dx = requestedWidth - targetWidth;

  const auto initialHeight = std::lround(
    std::ceil(
      std::ceil(GetIdealHeight(
        this->GetRoot()->GetLayoutNode(), targetWidth / mDPIScale))
      * mDPIScale));
  const auto requestedHeight
    = rect.bottom - (rect.top + ncPadding.top + ncPadding.bottom);
  const auto monitorHeight = monitorInfo.rcWork.bottom
    - (monitorInfo.rcWork.top + ncPadding.top + ncPadding.bottom);
  auto targetHeight = requestedHeight;
  switch (mVerticalResizeMode) {
    case ResizeMode::Fixed:
      targetHeight = initialHeight;
      break;
    case ResizeMode::AllowGrow:
      targetHeight = std::max(requestedHeight, initialHeight);
      break;
    case ResizeMode::AllowShrink:
      targetHeight = std::min(requestedHeight, initialHeight);
      targetHeight = std::min(targetHeight, monitorHeight);
      break;
    case ResizeMode::Allow:
      targetHeight = std::min(requestedHeight, monitorHeight);
      break;
  }
  const auto dy = requestedHeight - targetHeight;

  if (dx == 0 && dy == 0) {
    return {TRUE};
  }

  switch (wParam) {
    case WMSZ_TOPLEFT:
    case WMSZ_LEFT:
    case WMSZ_BOTTOMLEFT:
      rect.left += dx;
      break;
    default:
      rect.right -= dx;
      break;
  }

  switch (wParam) {
    case WMSZ_TOPLEFT:
    case WMSZ_TOP:
    case WMSZ_TOPRIGHT:
      rect.top += dy;
      break;
    default:
      rect.bottom -= dy;
  }

  mPendingResize = true;
  return {TRUE};
}

LRESULT
Win32Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (mHwnd && hwnd != mHwnd.get()) {
    throw std::logic_error("hwnd mismatch");
  }
  switch (uMsg) {
    case WM_ENABLE:
      mIsDisabled = !wParam;
      break;
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
      SystemSettings::Get().ClearWin32(static_cast<UINT>(wParam));
      break;
    case WM_SIZE: {
      if (wParam == SIZE_MINIMIZED) {
        break;
      }
      const auto w = LOWORD(lParam);
      const auto h = HIWORD(lParam);
      if (w != mClientSize.cx || h != mClientSize.cy) {
        mPendingResize = true;
      }
      return 0;
    }
    case WM_PAINT:
      this->Paint();
      break;
    case WM_SIZING: {
      if (const auto ret = WMSizingProc(wParam, lParam)) {
        return *ret;
      }
      break;
    }
    case WM_DPICHANGED: {
      const auto newDPI = HIWORD(wParam);
      // TODO: lParam is a RECT that we *should* use
      SetDPI(newDPI);
      break;
    }
    case WM_MOUSEACTIVATE: {
      if ((mOptions.mWindowExStyle & WS_EX_NOACTIVATE)) {
        return MA_NOACTIVATE;
      }
      break;
    }
    case WM_MOVE: {
      GetWindowRect(hwnd, &mNCRect);
      const auto x = LOWORD(lParam);
      const auto y = HIWORD(lParam);
      const auto dx = x - mPosition.mX;
      const auto dy = y - mPosition.mY;
      mPosition = {x, y};
      for (auto&& child: mChildren) {
        RECT rect {};
        GetWindowRect(child, &rect);
        SetWindowPos(
          child,
          nullptr,
          rect.left + dx,
          rect.top + dy,
          0,
          0,
          SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
      break;
    }
    case WM_MOUSEMOVE: {
      TrackMouseEvent();
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      if (const auto receiver = this->DispatchEvent(&e)) {
        switch (
          receiver->GetComputedStyle().Cursor().value_or(Cursor::Default)) {
          case Cursor::Default:
            SetCursor(mDefaultCursor.get());
            break;
          case Cursor::Pointer:
            SetCursor(mPointerCursor.get());
            break;
        }
      } else {
        SetCursor(mDefaultCursor.get());
      }
      break;
    }
    case WM_MOUSELEAVE: {
      MouseEvent e;
      e.mWindowPoint = {-1, -1};
      this->DispatchEvent(&e);
      mTrackingMouseEvents = false;
      break;
    }
    case WM_MOUSEWHEEL: {
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::VerticalWheelEvent {
        -static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA,
      };
      this->DispatchEvent(&e);
      break;
    }
    case WM_MOUSEHWHEEL: {
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::HorizontalWheelEvent {
        static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA,
      };
      this->DispatchEvent(&e);
    }
    case WM_LBUTTONDOWN: {
      for (auto&& child: mChildren) {
        gInstances.at(child)->RequestStop();
      }
      SetCapture(mHwnd.get());
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Left};
      this->DispatchEvent(&e);
      break;
    }
    case WM_LBUTTONUP: {
      ReleaseCapture();
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Left};
      this->DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONDOWN: {
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Middle};
      this->DispatchEvent(&e);
      break;
    }
    case WM_MBUTTONUP: {
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Middle};
      this->DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONDOWN: {
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Right};
      this->DispatchEvent(&e);
      break;
    }
    case WM_RBUTTONUP: {
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Right};
      this->DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONDOWN: {
      MouseButtons pressed {};
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        pressed |= MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        pressed |= MouseButton::X2;
      }
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonPressEvent {pressed};
      this->DispatchEvent(&e);
      break;
    }
    case WM_XBUTTONUP: {
      MouseButtons released {};
      if ((HIWORD(wParam) & XBUTTON1) == XBUTTON1) {
        released |= MouseButton::X1;
      }
      if ((HIWORD(wParam) & XBUTTON2) == XBUTTON2) {
        released |= MouseButton::X2;
      }
      auto e = MakeMouseEvent(wParam, lParam, mDPIScale);
      e.mDetail = MouseEvent::ButtonReleaseEvent {released};
      this->DispatchEvent(&e);
      break;
    }
    case WM_CLOSE:
      this->RequestStop();
      break;
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

SIZE Win32Window::GetInitialWindowSize() const {
  const auto contentSizeInDIPs = GetRoot()->GetInitialSize();

  RECT rect {
    0,
    0,
    std::lround(std::ceil(contentSizeInDIPs.mWidth * mDPIScale)),
    std::lround(std::ceil(contentSizeInDIPs.mHeight * mDPIScale)),
  };
  AdjustWindowRectExForDpi(
    &rect,
    mOptions.mWindowStyle & ~WS_OVERLAPPED,
    false,
    mOptions.mWindowExStyle,
    mDPI.value());
  this->ApplySizeConstraints(&rect);

  return {
    rect.right - rect.left,
    rect.bottom - rect.top,
  };
}

std::unique_ptr<Window> Win32Window::CreatePopup() const {
  return this->CreatePopup(
    GetModuleHandleW(nullptr),
    SW_SHOWNA,
    {
      .mWindowStyle = WS_POPUP | WS_BORDER,
      .mWindowExStyle = WS_EX_NOREDIRECTIONBITMAP | WS_EX_NOACTIVATE,
      .mSystemBackdrop = DWMSBT_TRANSIENTWINDOW,
      .mDXGIFactory = mDXGIFactory.get(),
    });
}

void Win32Window::WaitForInput() const {
  MsgWaitForMultipleObjects(0, nullptr, false, INFINITE, QS_ALLINPUT);
}

void Win32Window::SetIsModal(bool modal) {
  FUI_ASSERT(mParentHwnd);
  EnableWindow(mParentHwnd, !modal);
}

void Win32Window::SetResizeMode(
  const ResizeMode horizontal,
  const ResizeMode vertical) {
  if (horizontal == mHorizontalResizeMode && vertical == mVerticalResizeMode) {
    return;
  }
  mHorizontalResizeMode = horizontal;
  mVerticalResizeMode = vertical;
  auto rect = mNCRect;
  SendMessage(
    mHwnd.get(), WM_SIZING, WMSZ_BOTTOMRIGHT, reinterpret_cast<LPARAM>(&rect));
  if (memcmp(&rect, &mNCRect, sizeof(RECT)) == 0) {
    return;
  }
  SetWindowPos(
    mHwnd.get(),
    nullptr,
    rect.left,
    rect.top,
    rect.right - rect.left,
    rect.bottom - rect.top,
    SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOACTIVATE);
}

}// namespace FredEmmott::GUI