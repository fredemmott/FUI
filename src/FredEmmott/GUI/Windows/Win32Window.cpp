// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32Window.hpp"

#include <Windows.h>
#include <Windowsx.h>
#include <wil/win32_helpers.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>
#include <filesystem>

#ifdef FUI_ENABLE_SKIA
#include <FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.hpp>
#endif

namespace FredEmmott::GUI {
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

Win32Window::Win32Window(
  renderer_detail::RenderAPI renderApi,
  HINSTANCE hInstance,
  int nCmdShow,
  const Options& options)
  : Window(renderApi, SwapChainLength),
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
#ifdef FUI_ENABLE_SKIA
  return std::make_unique<Win32Direct3D12GaneshWindow>(
    hinstance, showCommand, options);
#endif
  return nullptr;
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
  const auto calculatedInitialSize = this->CalculateInitialWindowSize();
  mMinimumWidth = calculatedInitialSize.cx;
  SetWindowPos(
    mHwnd.get(),
    nullptr,
    0,
    0,
    calculatedInitialSize.cx,
    calculatedInitialSize.cy,
    SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
  RECT clientRect {};
  GetClientRect(mHwnd.get(), &clientRect);

  if (mOffsetToChild) {
    auto root = mOffsetToChild->GetLayoutNode();
    while (const auto node = YGNodeGetParent(root)) {
      root = node;
    }
    YGNodeCalculateLayout(
      root,
      static_cast<float>(clientRect.right - clientRect.left) / mDPIScale,
      static_cast<float>(clientRect.bottom - clientRect.top) / mDPIScale,
      YGDirectionLTR);
    const auto canvas = mOffsetToChild->GetTopLeftCanvasPoint();
    const auto native = CanvasPointToNativePoint(canvas);
    const auto nativeOrigin = mOptions.mInitialPosition;
    FUI_ASSERT(nativeOrigin.mX != CW_USEDEFAULT);
    FUI_ASSERT(nativeOrigin.mY != CW_USEDEFAULT);

    SetWindowPos(
      mHwnd.get(),
      nullptr,
      (2 * nativeOrigin.mX) - native.mX,
      (2 * nativeOrigin.mY) - native.mY,
      0,
      0,
      SWP_NOSIZE | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOZORDER
        | SWP_NOCOPYBITS);
  }

  GetWindowRect(mHwnd.get(), &mNCRect);
  GetClientRect(mHwnd.get(), &clientRect);
  mClientSize = {
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
  };
}

Win32Window::~Win32Window() {
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

void Win32Window::SetSystemBackdropType(DWM_SYSTEMBACKDROP_TYPE type) {
  FUI_ASSERT(
    !mHwnd, "Can't set system backdrop type after creating the window");
  mOptions.mSystemBackdrop = type;
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
    0, mClientSize.cx, mClientSize.cy, DXGI_FORMAT_UNKNOWN, 0));
  this->CreateRenderTargets();
}

void Win32Window::ResizeIfNeeded() {
  if (!mPendingResize.TestAndClear()) {
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

LRESULT
Win32Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
      SystemSettings::Get().ClearWin32(static_cast<UINT>(wParam));
      break;
    case WM_GETMINMAXINFO: {
      if (!mDPI) {
        this->SetDPI(GetDpiForWindow(hwnd));
      }
      if (!mMinimumWidth) {
        break;
      }
      auto* minInfo = reinterpret_cast<MINMAXINFO*>(lParam);
      minInfo->ptMinTrackSize.x = mMinimumWidth;
      return 0;
    }
    case WM_SIZE: {
      if (wParam == SIZE_MINIMIZED) {
        break;
      }
      const auto w = LOWORD(lParam);
      const auto h = HIWORD(lParam);
      if (w == mClientSize.cx && h == mClientSize.cy) {
        break;
      }
      mPendingResize.Set();
      break;
    }
    case WM_PAINT:
      this->Paint();
      break;
    case WM_SIZING: {
      // Initially this is the full window size, including the non-client
      // area
      RECT& rect = *reinterpret_cast<RECT*>(lParam);
      // Let's figure out how the client relates, and adjust from there
      RECT padding {};
      AdjustWindowRectEx(
        &padding, mOptions.mWindowStyle, false, mOptions.mWindowExStyle);
      padding.left = -padding.left;
      padding.top = -padding.top;
      const auto clientSize = SIZE {
        (rect.right - padding.right) - (rect.left + padding.left),
        (rect.bottom - padding.bottom) - (rect.top + padding.top),
      };

      const auto root = GetRoot();
      if (root->CanFit(
            std::floor(clientSize.cx / mDPIScale),
            std::floor(clientSize.cy / mDPIScale))) {
        mPendingResize.Set();
        break;
      }
      const auto height = std::ceil(
        root->GetHeightForWidth(std::floor(clientSize.cx / mDPIScale))
        * mDPIScale);
      if (rect.top == mNCRect.top) {
        rect.bottom = rect.top + padding.top + height + padding.bottom;
      } else {
        rect.top = rect.bottom - (padding.top + height + padding.bottom);
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
      this->DispatchEvent(&e);
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

SIZE Win32Window::CalculateInitialWindowSize() const {
  const auto contentSizeInDIPs = GetRoot()->GetInitialSize();

  RECT rect {
    0,
    0,
    std::lround(std::ceil(contentSizeInDIPs.mWidth * mDPIScale)),
    std::lround(std::ceil(contentSizeInDIPs.mHeight * mDPIScale)),
  };
  AdjustWindowRectEx(
    &rect,
    mOptions.mWindowStyle & ~WS_OVERLAPPED,
    false,
    mOptions.mWindowExStyle);

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

}// namespace FredEmmott::GUI