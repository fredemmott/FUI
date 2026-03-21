// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Win32Window.hpp"

#include <UIAutomationClient.h>
#include <UIAutomationCoreApi.h>
#include <Windows.h>
#include <Windowsx.h>
#include <roapi.h>
#include <wil/resource.h>
#include <wil/win32_helpers.h>
#include <yoga/yoga.h>

#include <FredEmmott/GUI/ExitException.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/Widgets/TitleBar.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/win32_detail.hpp>
#include <FredEmmott/GUI/detail/win32_detail/TSFTextStore.hpp>
#include <FredEmmott/GUI/detail/win32_detail/UIANode.hpp>
#include <FredEmmott/GUI/detail/win32_detail/UIARoot.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>
#include <FredEmmott/GUI/events/TextInputEvent.hpp>
#include <boost/container/static_vector.hpp>
#include <felly/numeric_cast.hpp>
#include <filesystem>
#include <print>

#include "FredEmmott/GUI/IconProvider.hpp"
#include "FredEmmott/GUI/events/HitTestEvent.hpp"

#ifdef FUI_ENABLE_SKIA
#include <FredEmmott/GUI/Windows/Win32Direct3D12GaneshWindow.hpp>
#endif
#ifdef FUI_ENABLE_DIRECT2D
#include <FredEmmott/GUI/Windows/Win32Direct2DWindow.hpp>
#endif

namespace FredEmmott::GUI {
using namespace win32_detail;

namespace {

constexpr bool DebugMouseMapping = false;

constexpr LiteralStyleClass ActualRootStyleClass {"Win32Window/Root"};
constexpr LiteralStyleClass ImmediateRootStyleClass {
  "Win32Window/ImmediateRoot"};
auto& ActualRootStyles() {
  static const ImmutableStyle ret {
    Style().FlexDirection(YGFlexDirectionColumn).FlexGrow(1),
  };
  return ret;
}
auto& ImmediateRootStyles() {
  static const ImmutableStyle ret {
    ActualRootStyles() + Style().FlexShrink(1).FlexGrow(1),
  };
  return ret;
}

auto GetApplicationHICON(const uint16_t edgeLength) {
  static ApplicationIconProvider provider;
  return provider.GetBestHICON(edgeLength);
}

thread_local Win32Window* gInstanceCreatingWindow {nullptr};
KeyCode KeyCodeFromVirtualKey(const UINT vk) {
  return static_cast<KeyCode>(vk);
}

KeyModifier GetModifierKeys() {
  BYTE keyState[256] {};
  GetKeyboardState(keyState);
  KeyModifier modifiers {};
  if (keyState[VK_SHIFT] & 0x80) {
    modifiers |= KeyModifier::Modifier_Shift;
  }
  if (keyState[VK_CONTROL] & 0x80) {
    modifiers |= KeyModifier::Modifier_Control;
  }
  if (keyState[VK_MENU] & 0x80) {
    modifiers |= KeyModifier::Modifier_Alt;
  }
  return modifiers;
}

std::wstring GetDefaultWindowClassName() {
  const auto thisExe = wil::QueryFullProcessImageNameW(GetCurrentProcess(), 0);
  return std::format(
    L"FredEmmott::GUI (FUI) Window - {}",
    std::filesystem::path(thisExe.get()).filename().wstring());
}

MouseEvent MakeMouseEvent(const WPARAM wParam, const Point canvasPoint) {
  MouseEvent ret;
  ret.mWindowPoint = canvasPoint;

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

void Win32Window::InitializeWidgetTree() {
  FUI_ASSERT(!mTitleBar);
  if (
    ((mOptions.mWindowStyle & WS_POPUP) == WS_POPUP)
    || !mOptions.mAllowModernTitleBar) {
    mActualRoot->SetStructuralChildren({mImmediateRoot});
  } else {
    mTitleBar = new Widgets::TitleBar(0);
    mTitleBar->SetTitle(mOptions.mTitle);
    mActualRoot->SetStructuralChildren({mTitleBar, mImmediateRoot});
  }
}

void Win32Window::InitializeWindow() {
  this->CreateNativeWindow();
  this->InitializeGraphicsAPI();
  this->InitializeDirectComposition();
  this->CreateRenderTargets();

  this->AdjustToWindowsTheme();

  if (
    (mOptions.mWindowExStyle & WS_EX_NOACTIVATE) == WS_EX_NOACTIVATE
    && mShowCommand == SW_SHOWDEFAULT) {
    mShowCommand = SW_SHOWNOACTIVATE;
  }

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
  : Win32Window(
      std::make_unique<Widgets::Widget>(
        0,
        ActualRootStyleClass,
        ActualRootStyles()),
      new Widgets::Widget(1, ImmediateRootStyleClass, ImmediateRootStyles()),
      hInstance,
      nCmdShow,
      options) {}

Win32Window::Win32Window(
  std::unique_ptr<Widgets::Widget> actualRoot,
  Widgets::Widget* immediateRoot,
  HINSTANCE hInstance,
  int nCmdShow,
  const Options& options)
  : Window(actualRoot.get(), immediateRoot, SwapChainLength),
    mActualRoot(std::move(actualRoot)),
    mImmediateRoot(immediateRoot),
    mInstanceHandle(hInstance),
    mShowCommand(nCmdShow),
    mOptions(options),
    mFrameIntervalTimer(CreateWaitableTimerExW(
      nullptr,
      nullptr,
      CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
      TIMER_ALL_ACCESS)),
    mWaitFrameInterruptEvent(CreateEventW(nullptr, FALSE, FALSE, nullptr)) {
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

void Win32Window::DestroyWindow() {
  FUI_ASSERT(mHwnd);
  mHwnd.reset();
}

void Win32Window::ProcessNativeEvents() {
  MSG msg {};
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (this->GetExitCode()) {
      return;
    }
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
  // Some launchers (e.g. debuggers) expect to be able to observe or stop
  // the process via the console
  std::ignore = AttachConsole(ATTACH_PARENT_PROCESS);

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
    case COMMode::WinRTSingleThreaded:
      CheckHResult(RoInitialize(RO_INIT_SINGLETHREADED));
      comCleanupFun = &RoUninitialize;
      break;
    case COMMode::WinRTMultiThreaded:
      CheckHResult(RoInitialize(RO_INIT_MULTITHREADED));
      comCleanupFun = &RoUninitialize;
      break;
  }

  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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

void Win32Window::TrackMouseEvent(const MouseTrackingArea where) {
  if (mTrackingMouseEvents) {
    return;
  }
  TRACKMOUSEEVENT tme {
    .cbSize = sizeof(tme),
    .dwFlags = TME_LEAVE | TME_HOVER,
    .hwndTrack = mHwnd.get(),
    .dwHoverTime = HOVER_DEFAULT,
  };
  if (where == MouseTrackingArea::NonClient) {
    tme.dwFlags |= TME_NONCLIENT;
  }
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

  const std::wstring title
    = mOptions.mTitle.empty() ? L"FUI Window" : Utf8ToWide(mOptions.mTitle);
  gInstanceCreatingWindow = this;
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
    this));
  FUI_ASSERT(!gInstanceCreatingWindow);
  if (!mHwnd) {
    CheckHResult(HRESULT_FROM_WIN32(GetLastError()));
    return;
  }
  FUI_ASSERT(GetWindowLongPtrW(mHwnd.get(), GWLP_USERDATA));
  if (mTitleBar) {
    const MARGINS margins {};
    CheckHResult(DwmExtendFrameIntoClientArea(mHwnd.get(), &margins));
  }

  if (mParentHwnd) {
    FUI_ASSERT(GetWindowLongPtrW(mParentHwnd, GWLP_USERDATA));
    Get(mParentHwnd).mChildren.push_back(mHwnd.get());
  }
  if ((mOptions.mWindowExStyle & WS_EX_LAYERED) == WS_EX_LAYERED) {
    SetLayeredWindowAttributes(mHwnd.get(), 0, 255, LWA_ALPHA);
  }

  this->UpdateGeometry();

  auto windowRect = mGeometry->mWindowRect;
  {
    const auto size = this->GetInitialWindowSize();
    windowRect.right = windowRect.left + size.cx;
    windowRect.bottom = windowRect.top + size.cy;
  }

  if (mOffsetToChild) {
    const auto yogaRoot = this->GetRoot()->GetLayoutNode();
#ifndef NDEBUG
    // We've needed `YGNodeSwapChild(yogaRoot, yogaChild, 1)` in the past,
    // but as the yoga functions now do that implicitly when operating on a
    // cloned node, this shouldn't be needed.
    //
    // e.g. past bug: https://github.com/fredemmott/FUI/issues/73
    FUI_ALWAYS_ASSERT(YGNodeGetChildCount(yogaRoot) == 1);
    const auto yogaChild = mActualRoot->GetLayoutNode();
    FUI_ALWAYS_ASSERT(YGNodeGetChild(yogaRoot, 0) == yogaChild);
    FUI_ASSERT(YGNodeGetParent(yogaChild) == yogaRoot);
#endif

    YGNodeCalculateLayout(
      yogaRoot, mGeometry->mCanvasSize.mWidth, YGUndefined, YGDirectionLTR);
    const auto canvas = mOffsetToChild->GetTopLeftCanvasPoint();
    const auto native = CanvasPointToNativePoint(canvas);
    const auto nativeOrigin = CanvasPointToNativePoint({0, 0});
    FUI_ASSERT(nativeOrigin.mX != CW_USEDEFAULT);
    FUI_ASSERT(nativeOrigin.mY != CW_USEDEFAULT);

    const auto dx = native.mX - nativeOrigin.mX;
    const auto dy = native.mY - nativeOrigin.mY;
    windowRect.left -= dx;
    windowRect.top -= dy;
    windowRect.right -= dx;
    windowRect.bottom -= dy;
  }

  std::ignore = this->ApplySizeConstraints(&windowRect);
  SetWindowPos(
    mHwnd.get(),
    nullptr,
    windowRect.left,
    windowRect.top,
    windowRect.right - windowRect.left,
    windowRect.bottom - windowRect.top,
    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

Win32Window::~Win32Window() {
  FUI_ASSERT(
    !mHwnd,
    "Window must be destroyed while vtable is alive; call DestroyWindow() in "
    "subclass destructor");
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

std::optional<std::string> Win32Window::GetClipboardText() const {
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    return {};
  }

  if (!OpenClipboard(mHwnd.get())) {
    return {};
  }
  const auto closeClipboard = wil::scope_exit([] { CloseClipboard(); });

  const auto handle = GetClipboardData(CF_UNICODETEXT);
  if (!handle) {
    return {};
  }

  const auto ptr = static_cast<wchar_t*>(GlobalLock(handle));
  if (!ptr) {
    return {};
  }
  const auto unlock = wil::scope_exit([handle] { GlobalUnlock(handle); });

  return WideToUtf8(ptr);
}

void Win32Window::SetClipboardText(const std::string_view utf8) const {
  if (!OpenClipboard(mHwnd.get())) {
    return;
  }
  const auto closeClipboard = wil::scope_exit([] { CloseClipboard(); });
  EmptyClipboard();

  const auto wide = Utf8ToWide(utf8);

  wil::unique_hglobal buffer {
    GlobalAlloc(GMEM_MOVEABLE, (wide.size() + 1) * sizeof(wchar_t))};

  {
    const auto ptr = static_cast<wchar_t*>(GlobalLock(buffer.get()));
    if (!ptr) {
      return;
    }
    memcpy(ptr, wide.data(), wide.size() * sizeof(wchar_t));
    ptr[wide.size()] = 0;
    GlobalUnlock(buffer.get());
  }

  SetClipboardData(CF_UNICODETEXT, buffer.get());
  // SetClipboardData takes ownership
  buffer.release();
}

void Win32Window::InitializeDirectComposition() {
  const bool haveDComp = SUCCEEDED(
    DCompositionCreateDevice(nullptr, IID_PPV_ARGS(mCompositionDevice.put())));

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
    .Width = static_cast<UINT>(
      mGeometry->mWindowRect.right - mGeometry->mWindowRect.left),
    .Height = static_cast<UINT>(
      mGeometry->mWindowRect.bottom - mGeometry->mWindowRect.top),
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {1, 0},
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = SwapChainLength,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode
    = haveDComp ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE,
    .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
  };

  if (haveDComp) {
    CheckHResult(mCompositionDevice->CreateTargetForHwnd(
      mHwnd.get(), true, mCompositionTarget.put()));
    CheckHResult(mCompositionDevice->CreateVisual(mCompositionVisual.put()));

    CheckHResult(mDXGIFactory->CreateSwapChainForComposition(
      this->GetDirectCompositionTargetDevice(),
      &swapChainDesc,
      nullptr,
      mSwapChain.put()));
    CheckHResult(mCompositionVisual->SetContent(mSwapChain.get()));
    CheckHResult(mCompositionTarget->SetRoot(mCompositionVisual.get()));
    CheckHResult(mCompositionDevice->Commit());
    return;
  }

  CheckHResult(mDXGIFactory->CreateSwapChainForHwnd(
    this->GetDirectCompositionTargetDevice(),
    mHwnd.get(),
    &swapChainDesc,
    nullptr,
    nullptr,
    mSwapChain.put()));
}

void Win32Window::ResizeSwapchain() {
  this->CleanupFrameContexts();
  CheckHResult(mSwapChain->ResizeBuffers(
    0,
    mGeometry->mWindowRect.right - mGeometry->mWindowRect.left,
    mGeometry->mWindowRect.bottom - mGeometry->mWindowRect.top,
    DXGI_FORMAT_UNKNOWN,
    DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));
  this->CreateRenderTargets();
  this->ResetToFirstBackBuffer();
}

void Win32Window::ResizeIfNeeded() {
  if (!std::exchange(mPendingResize, false)) {
    const auto yoga = this->GetRoot()->GetLayoutNode();
    if (!YGNodeGetHasNewLayout(yoga)) {
      return;
    }
    YGNodeSetHasNewLayout(yoga, false);
    const auto hadOverflow = YGNodeLayoutGetHadOverflow(yoga);
    if (!hadOverflow) {
      return;
    }
    const auto neededWidth = GetMinimumWidth(yoga, YGNodeLayoutGetWidth(yoga));
    const Rect canvasRect {
      Point {},
      Size {std::ceilf(neededWidth), mGeometry->mCanvasSize.mHeight},
    };
    auto screenRect = CanvasRectToScreenRect(canvasRect);
    std::ignore = ApplySizeConstraints(&screenRect);
    WINDOWPLACEMENT placement {sizeof(placement)};
    GetWindowPlacement(mHwnd.get(), &placement);
    placement.rcNormalPosition = screenRect;
    SetWindowPlacement(mHwnd.get(), &placement);
  }

  ResizeSwapchain();
}

Size Win32Window::GetCanvasSize() const {
  return mGeometry->mCanvasSize;
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
  RECT rect {};
  GetWindowRect(mHwnd.get(), &rect);
  if (!this->ApplySizeConstraints(&rect)) {
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
}

bool Win32Window::ApplySizeConstraints(RECT* ncrect) const {
  if (mIsToolTip) {
    // Keep tool tips in place, don't keep them within a single monitor
    return false;
  }
  bool modified = false;

  const_cast<Win32Window*>(this)->WMSizingProc(
    WMSZ_BOTTOMRIGHT, reinterpret_cast<LPARAM>(ncrect));

  const auto monitor = MonitorFromWindow(mHwnd.get(), MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo {sizeof(monitorInfo)};
  GetMonitorInfoW(monitor, &monitorInfo);
  if (ncrect->right > monitorInfo.rcWork.right) {
    modified = true;
    const auto dx = ncrect->right - monitorInfo.rcWork.right;
    ncrect->left -= dx;
    ncrect->right -= dx;
  }
  if (ncrect->bottom > monitorInfo.rcWork.bottom) {
    modified = true;
    const auto dy = ncrect->bottom - monitorInfo.rcWork.bottom;
    ncrect->bottom -= dy;
    ncrect->top -= dy;
  }

  return modified;
}

void Win32Window::OnDestroy() {
  if (!mHwnd) {
    return;
  }

  GetRoot()->Reset();
  this->CleanupFrameContexts();

  if (mParentHwnd) {
    auto& parent = Get(mParentHwnd);
    auto& siblings = parent.mChildren;
    siblings.erase(std::ranges::find(parent.mChildren, mHwnd.get()));
    mParentHwnd = nullptr;
  }

  for (auto&& child: mChildren) {
    Get(child).mParentHwnd = nullptr;
  }
}

void Win32Window::ResizeToIdeal() {
  RECT rect {};
  GetWindowRect(mHwnd.get(), &rect);
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

  const NativePoint native {
    felly::numeric_cast<int>(std::lround(canvas.mX * mDPIScale)),
    felly::numeric_cast<int>(std::lround(canvas.mY * mDPIScale)),
  };
  return native - mGeometry->mScreenToCanvasOffset;
}

Point Win32Window::NativePointToCanvasPoint(const NativePoint& native) const {
  FUI_ASSERT(mDPI && mHwnd);

  const auto [x, y] = native + mGeometry->mScreenToCanvasOffset;

  const Point canvas {
    static_cast<float>(x) / mDPIScale,
    static_cast<float>(y) / mDPIScale,
  };

  return canvas;
}

LRESULT Win32Window::StaticWindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam) {
  auto self
    = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) {
    if (uMsg == WM_NCCREATE) {
      // We have actual data, so let's use it :)
      const auto p = reinterpret_cast<CREATESTRUCT*>(lParam);
      self = static_cast<Win32Window*>(p->lpCreateParams);
      FUI_ASSERT(self == gInstanceCreatingWindow);
      if (gInstanceCreatingWindow == self) {
        gInstanceCreatingWindow = nullptr;
      }

      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(self));
    } else {
      // e.g. WM_GETMINMAXINFO can be sent before WM_NCCREATE, and does not
      // have the lParam
      self = gInstanceCreatingWindow;
    }
  }
  FUI_ASSERT(self);
  return self->WindowProc(hwnd, uMsg, wParam, lParam);
}

Win32Window& Win32Window::Get(HWND const hwnd) {
  const auto p
    = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  FUI_ASSERT(p);
  return *p;
}

void Win32Window::UpdateGeometry(const DWORD dpi) {
  mDPI = dpi ? dpi : GetDpiForWindow(mHwnd.get());
  mDPIScale = static_cast<float>(mDPI) / USER_DEFAULT_SCREEN_DPI;
  YGConfigSetPointScaleFactor(GetYogaConfig(), mDPIScale);
  mScreenMetrics = {mDPI};
  Size oldSize {};
  if (mGeometry) {
    oldSize = mGeometry->mCanvasSize;
  }
  using Kind = Geometry::Kind;
  mGeometry.emplace(
    mTitleBar ? Kind::WithNonClient : Kind::ClientAreaOnly,
    mHwnd.get(),
    mDPI,
    mDPIScale,
    mScreenMetrics);
  if (oldSize != mGeometry->mCanvasSize) {
    mPendingResize = true;
  }

  const auto iconSize = GetSystemMetricsForDpi(SM_CXICON, mDPI);
  if (auto icon = GetApplicationHICON(iconSize)) {
    SendMessageW(
      mHwnd.get(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon.get()));
    mIcon = std::move(icon);
  }
  const auto smallIconSize = GetSystemMetricsForDpi(SM_CXSMICON, mDPI);
  if (auto smallIcon = GetApplicationHICON(smallIconSize)) {
    SendMessageW(
      mHwnd.get(),
      WM_SETICON,
      ICON_SMALL,
      reinterpret_cast<LPARAM>(smallIcon.get()));
    mSmallIcon = std::move(smallIcon);
  }

  if (mTitleBar) {
    // Keep min/max/close visible, even if the content overflows horizontally
    mTitleBar->SetMutableStyles(Style().Width(mGeometry->mCanvasSize.mWidth));
  }
}

std::optional<LRESULT> Win32Window::WMSizingProc(WPARAM wParam, LPARAM lParam) {
  if (!mDPI) {
    return {};
  }

  if (
    mOptions.mHorizontalResizeMode == ResizeMode::Allow
    && mOptions.mVerticalResizeMode == ResizeMode::Allow) {
    return {};
  }

  RECT& rect = *reinterpret_cast<RECT*>(lParam);

  const auto monitor = MonitorFromWindow(mHwnd.get(), MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitorInfo {sizeof(monitorInfo)};
  GetMonitorInfoW(monitor, &monitorInfo);

  const auto& margins = mGeometry->mWindowMargins;
  const auto xMargins = margins.cxLeftWidth + margins.cxRightWidth;
  const auto yMargins = margins.cyTopHeight + margins.cyBottomHeight;

  const auto initialWidth = std::lround(
    std::ceil(
      std::ceil(GetMinimumWidth(this->GetRoot()->GetLayoutNode()))
      * mDPIScale));
  const auto requestedWidth = (rect.right - rect.left) - xMargins;
  const auto availableWidth
    = (monitorInfo.rcWork.right - monitorInfo.rcWork.left) - xMargins;
  auto targetWidth = requestedWidth;

  switch (mOptions.mHorizontalResizeMode) {
    case ResizeMode::Fixed:
      targetWidth = initialWidth;
      break;
    case ResizeMode::AllowGrow:
      targetWidth = std::max(requestedWidth, initialWidth);
      break;
    case ResizeMode::AllowShrink:
      targetWidth = std::min(requestedWidth, initialWidth);
      targetWidth = std::min(targetWidth, availableWidth);
      break;
    case ResizeMode::Allow:
      targetWidth = std::min(requestedWidth, availableWidth);
      break;
  }
  const auto dx = requestedWidth - targetWidth;

  const auto initialHeight = std::lround(
    std::ceil(
      std::ceil(GetIdealHeight(
        this->GetRoot()->GetLayoutNode(), targetWidth / mDPIScale))
      * mDPIScale));
  const auto requestedHeight = (rect.bottom - rect.top) - yMargins;
  const auto availableHeight
    = (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - yMargins;
  auto targetHeight = requestedHeight;
  switch (mOptions.mVerticalResizeMode) {
    case ResizeMode::Fixed:
      targetHeight = initialHeight;
      break;
    case ResizeMode::AllowGrow:
      targetHeight = std::max(requestedHeight, initialHeight);
      break;
    case ResizeMode::AllowShrink:
      targetHeight = std::min(requestedHeight, initialHeight);
      targetHeight = std::min(targetHeight, availableHeight);
      break;
    case ResizeMode::Allow:
      targetHeight = std::min(requestedHeight, availableHeight);
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

  if (LRESULT result {};
      DwmDefWindowProc(hwnd, uMsg, wParam, lParam, &result)) {
    return result;
  }

  if (const auto result = TitleBarWindowProc(hwnd, uMsg, wParam, lParam)) {
    return *result;
  }

  switch (uMsg) {
    case WM_NCDESTROY:
      // `wil::unique_hwnd::reset()` calls `DestroyWindow()`, but while we're
      // re-entrant in the message handler, the unique_hwnd should still be
      // live
      FUI_ASSERT(mHwnd.get() == hwnd);
      this->OnDestroy();
      // Explicitly call this to make sure we don't accidentally have any
      // references to hwnd (no longer valid, or we wouldn't have WM_NCDESTROY)
      return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    case WM_GETOBJECT: {
      if (static_cast<long>(lParam) == UiaRootObjectId) {
        if (!mUIAProvider) {
          mUIAProvider = wil::com_ptr_nothrow<IRawElementProviderFragmentRoot>(
            new win32_detail::UIARoot(this));
        }
        wil::com_ptr_nothrow<IRawElementProviderSimple> simple;
        mUIAProvider->QueryInterface(IID_PPV_ARGS(simple.put()));
        return UiaReturnRawElementProvider(hwnd, wParam, lParam, simple.get());
      }
      break;
    }
    case WM_ENABLE:
      mIsDisabled = !wParam;
      break;
    case WM_ACTIVATE:
      if (mTitleBar) {
        mTitleBar->SetIsActiveWindow(LOWORD(wParam) != WA_INACTIVE);
      }
      break;
    case WM_KEYDOWN: {
      const auto key = KeyCodeFromVirtualKey(wParam);
      KeyPressEvent e {key, GetModifierKeys()};
      this->DispatchEvent(e);
      break;
    }
    case WM_KEYUP: {
      const auto key = KeyCodeFromVirtualKey(wParam);
      KeyReleaseEvent e {key, GetModifierKeys()};
      this->DispatchEvent(e);
      break;
    }
    case WM_CHAR: {
      if (IS_HIGH_SURROGATE(wParam)) {
        mHighSurrogate = wParam;
        return 0;
      }
      const auto clearSurrogate
        = wil::scope_exit([this] { mHighSurrogate.reset(); });
      if (IS_LOW_SURROGATE(wParam)) {
        if (!mHighSurrogate) {
          __debugbreak();
          return 0;
        }
        if (IS_SURROGATE_PAIR(*mHighSurrogate, wParam)) {
          const wchar_t utf16[] {
            *std::move(mHighSurrogate),
            static_cast<wchar_t>(wParam),
          };
          const auto text = WideToUtf8(std::wstring_view {utf16, 2});
          TextInputEvent e {text};
          this->DispatchEvent(e);
          return 0;
        }
        __debugbreak();
        return 0;
      }
      if (wParam < 0x20) {
        // control characters/special keys, deal with these in WM_KEY messages
        // instead of here
        return 0;
      }
      const auto text = WideToUtf8(
        std::wstring_view {reinterpret_cast<const wchar_t*>(&wParam), 1});
      TextInputEvent e {text};
      this->DispatchEvent(e);
      break;
    }
    case WM_SETCURSOR: {
      const auto hitTest = LOWORD(lParam);
      if (hitTest == HTCLIENT) {
        switch (mWidgetCursorUnderMouse) {
          case Cursor::Default:
            SetCursor(mDefaultCursor.get());
            return true;
          case Cursor::Pointer:
            SetCursor(mPointerCursor.get());
            return true;
          case Cursor::Text:
            SetCursor(mTextCursor.get());
            return true;
        }
        std::unreachable();
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
      if (mTitleBar) {
        mTitleBar->SetIsMaximized(wParam == SIZE_MAXIMIZED);
      }

      UpdateGeometry();

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
      // TODO: lParam is a RECT that we *should* use
      FUI_ASSERT(
        LOWORD(wParam) == HIWORD(wParam),
        "Horizontal DPI must equal vertical DPI");
      UpdateGeometry(LOWORD(wParam));
      break;
    }
    case WM_MOUSEACTIVATE: {
      if ((mOptions.mWindowExStyle & WS_EX_NOACTIVATE)) {
        return MA_NOACTIVATE;
      }
      break;
    }
    case WM_MOVE: {
      this->UpdateGeometry();

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
      TrackMouseEvent(MouseTrackingArea::Client);
      const auto e = MakeMouseEventFromClient(wParam, lParam);
      if (const auto receiver = this->DispatchEvent(e)) {
        // Handled in response to WM_SETCURSOR
        mWidgetCursorUnderMouse
          = receiver->GetComputedStyle().Cursor().value_or(Cursor::Default);
      } else {
        mWidgetCursorUnderMouse = Cursor::Default;
      }
      break;
    }
    case WM_MOUSEHOVER: {
      // WM_MOUSEHOVER is a special, one-shot event; we need to set the flag
      // explicitly to force TrackMouseEvent() to actually re-do the call
      mTrackingMouseEvents = false;
      TrackMouseEvent(MouseTrackingArea::Client);
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::HoverEvent {};
      this->DispatchEvent(e);
      break;
    }
    case WM_MOUSELEAVE: {
      mTrackingMouseEvents = false;
      // TODO: only do this logic if we have a custom title bar
      POINT pt {};
      GetCursorPos(&pt);
      if (WindowFromPoint(pt) == mHwnd.get()) {
        // client -> non-client, we'll get WM_NCMOUSEMOVE or similar too
        // TODO: probably want more specific behavior based on WM_NCHITTEST
        break;
      }

      MouseEvent e;
      e.mWindowPoint = {-1, -1};
      this->DispatchEvent(e);
      break;
    }
    case WM_MOUSEWHEEL: {
      auto e = MakeMouseEventFromScreen(wParam, lParam);
      e.mDetail = MouseEvent::VerticalWheelEvent {
        -static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA,
      };
      this->DispatchEvent(e);
      break;
    }
    case WM_MOUSEHWHEEL: {
      auto e = MakeMouseEventFromScreen(wParam, lParam);
      e.mDetail = MouseEvent::HorizontalWheelEvent {
        static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA,
      };
      this->DispatchEvent(e);
      break;
    }
    case WM_LBUTTONDOWN: {
      for (auto&& child: mChildren) {
        Get(child).RequestStop();
      }
      SetCapture(mHwnd.get());
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Left};
      this->DispatchEvent(e);
      break;
    }
    case WM_LBUTTONUP: {
      ReleaseCapture();
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Left};
      this->DispatchEvent(e);
      break;
    }
    case WM_MBUTTONDOWN: {
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Middle};
      this->DispatchEvent(e);
      break;
    }
    case WM_MBUTTONUP: {
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Middle};
      this->DispatchEvent(e);
      break;
    }
    case WM_RBUTTONDOWN: {
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Right};
      this->DispatchEvent(e);
      break;
    }
    case WM_RBUTTONUP: {
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Right};
      this->DispatchEvent(e);
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
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonPressEvent {pressed};
      this->DispatchEvent(e);
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
      auto e = MakeMouseEventFromClient(wParam, lParam);
      e.mDetail = MouseEvent::ButtonReleaseEvent {released};
      this->DispatchEvent(e);
      break;
    }
    case WM_CLOSE:
      this->RequestStop();
      break;
    default:
      break;
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
std::optional<LRESULT> Win32Window::TitleBarWindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam) {
  if (!mTitleBar) {
    return std::nullopt;
  }

  if (YGNodeStyleGetDisplay(mTitleBar->GetLayoutNode()) == YGDisplayNone) {
    return std::nullopt;
  }

  // TITLEBARINFOEX indices
  // static constexpr auto TitleBarIdx = 0;
  // RESERVED = 1
  static constexpr auto MinimizeIdx = 2;
  static constexpr auto MaximizeIdx = 3;
  static constexpr auto HelpIdx = 4;
  static constexpr auto CloseIdx = 5;

  switch (uMsg) {
    case WM_NCMOUSEHOVER:
      mTrackingMouseEvents = false;
      TrackMouseEvent(MouseTrackingArea::NonClient);
      return std::nullopt;
    case WM_NCMOUSEMOVE: {
      TrackMouseEvent(MouseTrackingArea::NonClient);
      const auto e = MakeMouseEventFromScreen(wParam, lParam);
      this->DispatchEvent(e);
      return 0;
    }
    case WM_NCLBUTTONDOWN:
      switch (wParam) {
        case HTMINBUTTON:
        case HTMAXBUTTON:
        case HTCLOSE: {
          auto e = MakeMouseEventFromScreen(wParam, lParam);
          e.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Left};
          this->DispatchEvent(e);
          return 0;
        }
        default:
          return DefWindowProcW(hwnd, uMsg, wParam, lParam);
      }
    case WM_NCLBUTTONUP: {
      auto e = MakeMouseEventFromScreen(wParam, lParam);
      e.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Left};
      this->DispatchEvent(e);

      if (mTitleBar->ConsumeWasMinimizeActivated()) {
        PostMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
      }
      if (mTitleBar->ConsumeWasMaximizeActivated()) {
        if (IsZoomed(hwnd)) {
          PostMessageW(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
        } else {
          PostMessageW(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
      }
      if (mTitleBar->ConsumeWasCloseActivated()) {
        PostMessageW(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
      }

      return 0;
    }
    case WM_NCMOUSELEAVE: {
      mTrackingMouseEvents = false;
      POINT pt {};
      GetCursorPos(&pt);
      if (WindowFromPoint(pt) == mHwnd.get()) {
        // We moved from non-client to client, so we'll have a regular
        // WM_MOUSEMOVE too
        return 0;
      }
      // Entirely left the window, so we need to act on it
      MouseEvent e;
      e.mWindowPoint = {-1, -1};
      e.mDetail = MouseEvent::MoveEvent {};
      this->DispatchEvent(e);
      return 0;
    }
    case WM_NCHITTEST: {
      const POINT pt {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      if (!IsZoomed(hwnd)) {
        const auto& window = mGeometry->mWindowRect;
        const auto frameX
          = mScreenMetrics.mFrameX + mScreenMetrics.mPaddedBorder;
        const auto frameTop = mScreenMetrics.mFrameY;
        const auto frameBottom = frameTop + mScreenMetrics.mPaddedBorder;

        // ----- Borders -----
        const bool isTop = pt.y <= window.top + frameTop;
        const bool isBottom = pt.y >= window.bottom - frameBottom;
        const bool isLeft = pt.x <= window.left + frameX;
        const bool isRight = pt.x >= window.right - frameX;

        if (isTop && isLeft)
          return HTTOPLEFT;
        if (isTop && isRight)
          return HTTOPRIGHT;
        if (isBottom && isLeft)
          return HTBOTTOMLEFT;
        if (isBottom && isRight)
          return HTBOTTOMRIGHT;

        if (isTop)
          return HTTOP;
        if (isBottom)
          return HTBOTTOM;
        if (isLeft)
          return HTLEFT;
        if (isRight)
          return HTRIGHT;
      }

      // ----- Title bar-----
      const auto rects = mTitleBar->GetRects();
      const auto titleBar = CanvasRectToScreenRect(rects.mFullArea);
      if (!PtInRect(&titleBar, pt)) {
        return HTCLIENT;
      }
      const auto icon = CanvasRectToScreenRect(rects.mIconButton);
      if (PtInRect(&icon, pt)) {
        return HTSYSMENU;
      }

      const auto min = CanvasRectToScreenRect(rects.mMinimizeButton);
      if (PtInRect(&min, pt)) {
        return HTMINBUTTON;
      }
      const auto max = CanvasRectToScreenRect(rects.mMaximizeButton);
      if (PtInRect(&max, pt)) {
        return HTMAXBUTTON;
      }
      const auto close = CanvasRectToScreenRect(rects.mCloseButton);
      if (PtInRect(&close, pt)) {
        return HTCLOSE;
      }

      HitTestEvent hte {};
      hte.mPoint = NativePointToCanvasPoint(NativePoint {pt.x, pt.y});
      const auto w = GetRoot()->DispatchEvent(hte);
      if (dynamic_cast<Widgets::IFocusable*>(w)) {
        return HTCLIENT;
      }

      return HTCAPTION;
    }
    case WM_GETTITLEBARINFOEX: {
      const auto ret = reinterpret_cast<TITLEBARINFOEX*>(lParam);
      FUI_ASSERT(ret->cbSize >= sizeof(TITLEBARINFOEX));
      *ret = {sizeof(*ret)};

      const auto canvasRects = mTitleBar->GetRects();
      FUI_ASSERT(canvasRects.mFullArea.mTopLeft == Point {});
      ret->rcTitleBar = CanvasRectToScreenRect(canvasRects.mFullArea);

      const auto hovered = mTitleBar->GetHoveredButton();

      using enum Widgets::TitleBar::ChromeButton;
      for (auto&& [idx, button, canvasRef]: {
             std::tuple {
               MinimizeIdx, Minimize, std::ref(canvasRects.mMinimizeButton)},
             std::tuple {
               MaximizeIdx, Maximize, std::ref(canvasRects.mMaximizeButton)},
             std::tuple {CloseIdx, Close, std::ref(canvasRects.mCloseButton)},
           }) {
        auto& screenRect = ret->rgrect[idx];
        const auto& canvasRect = canvasRef.get();
        screenRect = CanvasRectToScreenRect(canvasRect);

        if (hovered == button) {
          ret->rgstate[idx] |= STATE_SYSTEM_HOTTRACKED;
        }
      }

      ret->rgstate[HelpIdx] = STATE_SYSTEM_OFFSCREEN | STATE_SYSTEM_UNAVAILABLE;

      return 0;
    }
    case WM_NCCALCSIZE: {
      if (wParam) {
        this->UpdateGeometry();
        auto params = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam);
        //  In:
        //
        //  0. Window coordinates
        //  1. Old window coordinates
        //  2. Old client coordinates
        //
        //  Out:
        //
        //  0. New client rect
        //  1. CopyTo
        //  2. CopyFrom

        params->rgrc[1] = params->rgrc[0];

        const auto& margins = mGeometry->mWindowMargins;
        params->rgrc[0].top += margins.cyTopHeight;
        params->rgrc[0].bottom -= margins.cyBottomHeight;
        params->rgrc[0].left += margins.cxLeftWidth;
        params->rgrc[0].right -= margins.cxRightWidth;

        return 0;
      }
      return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    default:
      return std::nullopt;
  }

  FUI_FATAL(
    "Window message {} unhandled; call DefWindowProc or return std::nullopt",
    uMsg);
}

SIZE Win32Window::GetInitialWindowSize() const {
  const auto ToPixels
    = [scale = mDPIScale](const float v) { return std::lround(v * scale); };
  const auto canvas = GetRoot()->GetInitialSize();

  const auto [l, r, t, b] = mGeometry->mWindowMargins;
  RECT rect {
    0,
    0,
    ToPixels(canvas.mWidth) + l + r,
    ToPixels(canvas.mHeight) + t + b,
  };
  std::ignore = this->ApplySizeConstraints(&rect);

  return {
    rect.right - rect.left,
    rect.bottom - rect.top,
  };
}

RECT Win32Window::CanvasRectToScreenRect(const Rect& canvasRect) const {
  const auto topLeft = CanvasPointToNativePoint(canvasRect.GetTopLeft());
  const auto bottomRight
    = CanvasPointToNativePoint(canvasRect.GetBottomRight());
  return RECT {
    topLeft.mX,
    topLeft.mY,
    bottomRight.mX,
    bottomRight.mY,
  };
}

std::unique_ptr<Window> Win32Window::CreatePopup() const {
  return this->CreatePopup(
    GetModuleHandleW(nullptr),
    SW_SHOWDEFAULT,
    {
      .mWindowStyle = WS_POPUP | WS_BORDER,
      .mWindowExStyle = WS_EX_NOREDIRECTIONBITMAP,
      .mSystemBackdrop = DWMSBT_TRANSIENTWINDOW,
      .mDXGIFactory = mDXGIFactory.get(),
    });
}

void Win32Window::SetIsToolTip() {
  this->MutateStyles([]([[maybe_unused]] DWORD* styles, DWORD* extendedStyles) {
    *extendedStyles |= WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT
      | WS_EX_LAYERED;
  });
  mIsToolTip = true;
}

void Win32Window::WaitFrameImpl(
  const std::span<const NativeWaitable> callerHandles,
  const std::chrono::steady_clock::time_point until) const {
  using HighResolutionTimerTicks
    = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;
  using time_point = std::remove_cvref_t<decltype(until)>;
  using clock = time_point::clock;

  // callerHandles + interrupt handle + timer handle
  if (callerHandles.size() + 2 > MAXIMUM_WAIT_OBJECTS) {
    throw std::runtime_error(
      std::format(
        "Windows can only wait for up to {} objects; need to wait on {} events "
        "({} caller-provided)",
        MAXIMUM_WAIT_OBJECTS,
        callerHandles.size() + 2,
        callerHandles.size()));
  }
  boost::container::static_vector<HANDLE, MAXIMUM_WAIT_OBJECTS> handles;
  std::ranges::copy(
    std::views::transform(callerHandles, &NativeWaitable::mHandle),
    std::back_inserter(handles));
  handles.push_back(mWaitFrameInterruptEvent.get());

  if (const auto now = clock::now();
      until != time_point::max() && until > now) {
    /* Using SetWaitableTimer because the standard timeout isn't high enough
     * resolution to maintain a reasonable approximation of 60hz.
     *
     * A negative `timeout` is a duration, whereas a positive is a specific
     * timepoint.
     *
     * Despite having an absolute target time, using negative value here to
     * id needing to convert between `steady_clock` and `system_clock`
     */
    const auto duration = until - now;
    const LARGE_INTEGER timeout {
      .QuadPart
      = -std::chrono::duration_cast<HighResolutionTimerTicks>(duration).count(),
    };
    if (!SetWaitableTimer(
          mFrameIntervalTimer.get(), &timeout, 0, nullptr, nullptr, false)) {
      throw std::runtime_error(
        std::format(
          "Failed to create waitable timer: {}",
          static_cast<int>(HRESULT_FROM_WIN32(GetLastError()))));
    }
    handles.push_back(mFrameIntervalTimer.get());
  }

  MsgWaitForMultipleObjects(
    static_cast<DWORD>(handles.size()),
    handles.data(),
    FALSE,
    INFINITE,
    QS_ALLINPUT);

  CancelWaitableTimer(mFrameIntervalTimer.get());
  ResetEvent(mWaitFrameInterruptEvent.get());
}

Win32Window::ScreenMetrics::ScreenMetrics(const DWORD dpi) {
  const auto gsm
    = [dpi](const int idx) { return GetSystemMetricsForDpi(idx, dpi); };
  mFrameX = gsm(SM_CXFRAME);
  mFrameY = gsm(SM_CYFRAME);
  mPaddedBorder = gsm(SM_CXPADDEDBORDER);
}

Win32Window::Geometry::Geometry(
  const Kind kind,
  HWND const hwnd,
  const DWORD dpi,
  const float dpiScale,
  const ScreenMetrics& metrics) {
  mDPI = dpi;
  mDPIScale = dpiScale;
  GetWindowRect(hwnd, &mWindowRect);
  POINT clientOrigin {};
  ClientToScreen(hwnd, &clientOrigin);
  mClientToScreenOffset = {clientOrigin.x, clientOrigin.y};

  if (kind == Kind::ClientAreaOnly) {
    this->InitializeForClientCanvas(hwnd, metrics);
  } else {
    this->InitializeForNonClientCanvas(hwnd, metrics);
  }
}

void Win32Window::Geometry::InitializeForClientCanvas(
  HWND const hwnd,
  const ScreenMetrics&) {
  mScreenToCanvasOffset = {
    -mClientToScreenOffset.mX,
    -mClientToScreenOffset.mY,
  };
  RECT clientRect {};
  GetClientRect(hwnd, &clientRect);
  mCanvasSize = {
    static_cast<float>(clientRect.right - clientRect.left) / mDPIScale,
    static_cast<float>(clientRect.bottom - clientRect.top) / mDPIScale,
  };

  mWindowMargins.cxLeftWidth = mClientToScreenOffset.mX - mWindowRect.left;
  mWindowMargins.cyTopHeight = mClientToScreenOffset.mY - mWindowRect.top;

  const auto clientRight = mClientToScreenOffset.mX + clientRect.right;
  mWindowMargins.cxRightWidth = mWindowRect.right - clientRight;
  const auto clientBottom = mClientToScreenOffset.mY + clientRect.bottom;
  mWindowMargins.cyBottomHeight = mWindowRect.bottom - clientBottom;
}

void Win32Window::Geometry::InitializeForNonClientCanvas(
  HWND const hwnd,
  const ScreenMetrics& metrics) {
  const auto windowWidth = mWindowRect.right - mWindowRect.left;
  const auto windowHeight = mWindowRect.bottom - mWindowRect.top;

  if (IsZoomed(hwnd)) {
    const auto pad = metrics.mPaddedBorder;
    mWindowMargins = {
      metrics.mFrameX + pad,
      metrics.mFrameX + pad,
      metrics.mFrameY + pad,
      metrics.mFrameY + pad,
    };
  } else {
    mWindowMargins = {metrics.mFrameX, metrics.mFrameX, 0, metrics.mFrameY};
  }

  const auto horizontalMargins
    = mWindowMargins.cxLeftWidth + mWindowMargins.cxRightWidth;
  const auto verticalMargins
    = mWindowMargins.cyTopHeight + mWindowMargins.cyBottomHeight;

  mScreenToCanvasOffset = {
    -(mWindowRect.left + mWindowMargins.cxLeftWidth),
    -(mWindowRect.top + mWindowMargins.cyTopHeight),
  };
  mCanvasSize = {
    static_cast<float>(windowWidth - horizontalMargins) / mDPIScale,
    static_cast<float>(windowHeight - verticalMargins) / mDPIScale,
  };
}

void Win32Window::SetIsModal(const bool modal) {
  FUI_ASSERT(mParentHwnd);
  EnableWindow(mParentHwnd, !modal);
}

void Win32Window::SetResizeMode(
  const ResizeMode horizontal,
  const ResizeMode vertical) {
  if (
    horizontal == mOptions.mHorizontalResizeMode
    && vertical == mOptions.mVerticalResizeMode) {
    return;
  }
  mOptions.mHorizontalResizeMode = horizontal;
  mOptions.mVerticalResizeMode = vertical;

  if (!mHwnd) {
    return;
  }
  FUI_ASSERT(mGeometry);

  const auto oldRect = mGeometry->mWindowRect;
  auto rect = oldRect;
  SendMessage(
    mHwnd.get(), WM_SIZING, WMSZ_BOTTOMRIGHT, reinterpret_cast<LPARAM>(&rect));
  if (memcmp(&rect, &oldRect, sizeof(RECT)) == 0) {
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

void Win32Window::MutateStyles(
  void (*mutator)(DWORD* styles, DWORD* extendedStyles)) {
  auto styles = mOptions.mWindowStyle;
  auto extendedStyles = mOptions.mWindowExStyle;
  mutator(&styles, &extendedStyles);
  if (mOptions.mWindowStyle != styles) {
    mOptions.mWindowStyle = styles;
    SetWindowLongPtrW(mHwnd.get(), GWL_STYLE, styles);
  }
  if (mOptions.mWindowExStyle != extendedStyles) {
    mOptions.mWindowExStyle = extendedStyles;
    SetWindowLongPtrW(mHwnd.get(), GWL_EXSTYLE, extendedStyles);
  }

  if (!mHwnd.get()) {
    return;
  }

  SetWindowPos(
    mHwnd.get(),
    nullptr,
    0,
    0,
    0,
    0,
    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

  if ((mOptions.mWindowExStyle & WS_EX_LAYERED) == WS_EX_LAYERED) {
    SetLayeredWindowAttributes(mHwnd.get(), 0, 255, LWA_ALPHA);
  }
}

void Win32Window::InterruptWaitFrame() {
  SetEvent(mWaitFrameInterruptEvent.get());
}

void Win32Window::SetTitle(const std::string_view text) {
  if (text == mOptions.mTitle) {
    return;
  }

  mOptions.mTitle = text;
  if (mTitleBar) {
    mTitleBar->SetTitle(text);
  }
  SetWindowTextW(mHwnd.get(), Utf8ToWide(text).c_str());
}

bool Win32Window::SetSubtitle(const std::string_view text) {
  if (!mTitleBar) {
    return false;
  }
  mTitleBar->SetSubtitle(text);
  return true;
}

MouseEvent Win32Window::MakeMouseEventFromScreen(
  const WPARAM wParam,
  const POINT& screenPoint) {
  const auto [dx, dy] = mGeometry->mScreenToCanvasOffset;
  const Point canvasPoint {
    (screenPoint.x + dx) / mDPIScale,
    (screenPoint.y + dy) / mDPIScale,
  };
  if constexpr (DebugMouseMapping) {
    const auto roundTrip = CanvasPointToNativePoint(canvasPoint);
    FUI_ALWAYS_ASSERT(roundTrip.mX == screenPoint.x);
    FUI_ALWAYS_ASSERT(roundTrip.mY == screenPoint.y);
  }
  return MakeMouseEvent(wParam, canvasPoint);
}

MouseEvent Win32Window::MakeMouseEventFromScreen(
  const WPARAM wParam,
  const LPARAM lParam) {
  return MakeMouseEventFromScreen(
    wParam, POINT {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
}

MouseEvent Win32Window::MakeMouseEventFromClient(WPARAM wParam, LPARAM lParam) {
  POINT screenPoint {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  screenPoint.x += mGeometry->mClientToScreenOffset.mX;
  screenPoint.y += mGeometry->mClientToScreenOffset.mY;
  return MakeMouseEventFromScreen(wParam, screenPoint);
}

}// namespace FredEmmott::GUI
