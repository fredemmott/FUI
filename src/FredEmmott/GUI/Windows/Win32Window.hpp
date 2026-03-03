// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <Windows.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <uiautomationcore.h>
#include <wil/com.h>

#include <FredEmmott/GUI/Immediate/Root.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <chrono>
#include <optional>

#include "FredEmmott/GUI/Point.hpp"
#include "FredEmmott/memory.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
class TitleBar;
}// namespace FredEmmott::GUI::Widgets

namespace FredEmmott::GUI {
struct WindowOptions {
  std::string mTitle;

  NativePoint mInitialPosition {CW_USEDEFAULT, CW_USEDEFAULT};

  std::string mClass;
  HWND mParentWindow {nullptr};

  DWORD mWindowStyle {WS_OVERLAPPEDWINDOW};
  DWORD mWindowExStyle {WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP};

  bool mAllowModernTitleBar = true;

  DWM_SYSTEMBACKDROP_TYPE mSystemBackdrop {DWMSBT_MAINWINDOW};

  IDXGIFactory* mDXGIFactory {nullptr};
};

class Win32Window;

struct WinMainOptions {
  struct Hooks {
    void (*mBeforeWindow)() {nullptr};
    unique_ptr<Win32Window> (*mCreateWindow)(
      HINSTANCE instance,
      UINT showCommand,
      const WindowOptions&) {nullptr};
    void (*mBeforeMainLoop)(Win32Window&) {nullptr};
    void (*mAfterMainLoop)(Win32Window&, int exitCode) {nullptr};
  };

  enum class COMMode {
    Uninitialized,
    WinRTSingleThreaded,
    WinRTMultiThreaded,// implies ApartmentThreaded
  };
  enum class COMCleanupMode {
    None,
    Uninitialize,
  };
  COMMode mCOMMode {COMMode::WinRTSingleThreaded};
  COMCleanupMode mCOMCleanupMode {COMCleanupMode::Uninitialize};

  Hooks mHooks {};
};

class Win32Window : public Window {
 public:
  using Options = WindowOptions;

  Win32Window() = delete;
  Win32Window(const Win32Window&) = delete;
  Win32Window(Win32Window&&) = delete;
  Win32Window& operator=(const Win32Window&) = delete;
  Win32Window& operator=(Win32Window&&) = delete;

  static unique_ptr<Win32Window>
  CreateAny(HINSTANCE hinstance, int showCommand, const Options& options = {});

  /** Main loop; continues until a stop is requested.
   *
   * Can be requested by:
   * - throwing an `ExitException`
   * - calling `Window::RequestStop()`
   */
  [[nodiscard]]
  static int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow,
    void (*appTick)(Win32Window&),
    const WindowOptions& windowOptions = {},
    const WinMainOptions& options = {});

  ~Win32Window() override;

  void SetSystemBackdropType(DWM_SYSTEMBACKDROP_TYPE);

  [[nodiscard]]
  NativeHandle GetNativeHandle() const noexcept final {
    return {mHwnd ? mHwnd.get() : nullptr};
  }

  [[nodiscard]] std::string_view GetTitle() const noexcept {
    return mOptions.mTitle;
  }

  std::optional<std::string> GetClipboardText() const override;
  void SetClipboardText(std::string_view) const override;

  void SetParent(NativeHandle) final;
  void SetInitialPositionInNativeCoords(const NativePoint& native) final;
  void OffsetPositionToDescendant(Widgets::Widget* child) final;
  void ApplySizeConstraints();
  void ResizeToIdeal() override;

  NativePoint CanvasPointToNativePoint(const Point& canvas) const final;
  Point NativePointToCanvasPoint(const NativePoint& native) const final;

  std::unique_ptr<Window> CreatePopup() const final;

  void SetIsModal(bool modal);
  void SetResizeMode(ResizeMode horizontal, ResizeMode vertical) override;

  void MutateStyles(void (*)(DWORD* styles, DWORD* extendedStyles));

  void InterruptWaitFrame() override;

  void SetTitle(std::string_view) override;
  [[nodiscard]] bool SetSubtitle(std::string_view) override;

 protected:
  static constexpr UINT SwapChainLength = 3;

  Win32Window(HINSTANCE instance, int showCommand, const Options& options);

  void DestroyWindow();

  void ProcessNativeEvents() override;
  void InitializeWindow() final;
  void HideWindow() final;
  void ResizeIfNeeded() final;
  Size GetCanvasSize() const final;
  float GetDPIScale() const final;
  Color GetClearColor() const final;
  void InitializeWidgetTree() override;

  virtual std::unique_ptr<Win32Window> CreatePopup(
    HINSTANCE instance,
    int showCommand,
    const Options& options) const = 0;
  virtual IUnknown* GetDirectCompositionTargetDevice() const = 0;
  virtual void CreateRenderTargets() = 0;
  virtual void CleanupFrameContexts() = 0;
  virtual void OnDestroy();

  auto GetDXGIFactory() const noexcept {
    return mDXGIFactory.get();
  }

  auto GetSwapChain() const noexcept {
    return mSwapChain.get();
  }

  bool IsPopup() const noexcept override {
    return (mOptions.mWindowStyle & WS_POPUP)
      || (mOptions.mWindowExStyle & WS_EX_TOOLWINDOW);
  }

  void SetIsToolTip() override;

  bool IsDisabled() const override {
    return mIsDisabled;
  }

  void WaitFrameImpl(
    std::span<const NativeWaitable>,
    std::chrono::steady_clock::time_point until) const final;

 private:
  enum class MouseTrackingArea {
    Client,
    NonClient,
  };
  template <class T>
  struct Dimensions {};
  struct ScreenMetrics {
    ScreenMetrics() = delete;
    ScreenMetrics(DWORD dpi);
    long mFrameX {};
    long mFrameY {};
    long mPaddedBorder {};
  };
  /** Conversions between coordinate systems.
   *
   * Windows gives us two different coordinate systems, and FUI expects them
   * to be in physical pixels:
   *
   * - Screen coordinates: (0, 0) is at the top left of the primary monitor;
   *   negative coordinates are possible
   * - Client coordinates: (0, 0) is at the top left of the client area of the
   *   window, which is generally the 'usable' area - it excludes title bar,
   *   borders, etc
   *
   * Then there's the canvas system:
   *
   * - (0, 0) is wherever we can actually paint
   * - it is in "Device Independent Pixels" (DIPs); for the win32 part of
   *   things, we are given a DPI, and we compare it to
   *   `USER_DEFAULT_SCREEN_DPI`; once we reach Widgets or Yoga though,
   *   coordinates are float, so we use a float 'DPI scale' instead
   * - to convert between the others, we need:
   *   - an offset (in physical pixels)
   *   - the DPI scale
   *
   * If we have no title bar or the default windows title bar, canvas (0, 0)
   * is client (0, 0); but if we have a custom title bar (non-client area), it's
   * slightly inside; the math here also depends on whether we're maximized or
   * not.
   *
   * Even if we're sticking with the client area, scroll wheel events are in
   * screen coordinates, and all non-client mouse events are in screen
   * coordinates - so, regardless of which 'mode' we're in, we'll:
   *
   * - normalize mouse input to screen coordinates (so we need a
   *   client-to-screen offset)
   * - once we have screen coordinates, normalize them again to 'canvas'
   *   coordinates
   *
   * Also, Yoga can figure out how big our canvas needs to be; we need to figure
   * out how big the Window needs to be to fit that canvas, i.e. how much
   * padding is needed.
   */
  struct Geometry {
    enum class Kind {
      WithNonClient,
      ClientAreaOnly,
    };
    Geometry() = delete;
    explicit Geometry(
      Kind kind,
      HWND,
      DWORD dpi,
      float dpiScale,
      const ScreenMetrics&);

    DWORD mDPI {};
    float mDPIScale {};

    RECT mWindowRect {};
    MARGINS mWindowMargins {};
    NativePoint mClientToScreenOffset {};
    NativePoint mScreenToCanvasOffset {};

    Size mCanvasSize {};

   private:
    void InitializeForNonClientCanvas(HWND, const ScreenMetrics&);
    void InitializeForClientCanvas(HWND, const ScreenMetrics&);
  };

  std::unique_ptr<Widgets::Widget> mActualRoot;
  Widgets::TitleBar* mTitleBar {nullptr};
  Widgets::Widget* mImmediateRoot {nullptr};

  DWORD mDPI {USER_DEFAULT_SCREEN_DPI};
  float mDPIScale {1.0f};
  ScreenMetrics mScreenMetrics {USER_DEFAULT_SCREEN_DPI};
  std::optional<Geometry> mGeometry;

  wil::com_ptr<IRawElementProviderFragmentRoot> mUIAProvider;
  HINSTANCE mInstanceHandle {nullptr};
  int mShowCommand {SW_SHOW};
  Options mOptions {};

  wil::unique_event mFrameIntervalTimer;
  wil::unique_event mWaitFrameInterruptEvent;

  HWND mParentHwnd {nullptr};
  wil::unique_hwnd mHwnd;
  std::vector<HWND> mChildren;

  bool mIsToolTip {false};
  float mMinimumCanvasWidth {};
  Widgets::Widget* mOffsetToChild {nullptr};
  bool mPendingResize {false};
  bool mTrackingMouseEvents {false};
  std::optional<uint16_t> mHighSurrogate;
  NativePoint mPosition {};
  ResizeMode mHorizontalResizeMode = ResizeMode::AllowGrow;
  ResizeMode mVerticalResizeMode = ResizeMode::Fixed;

  wil::com_ptr<IDXGIFactory4> mDXGIFactory;
  wil::com_ptr<IDXGISwapChain1> mSwapChain;

  wil::com_ptr<IDCompositionDevice> mCompositionDevice;
  wil::com_ptr<IDCompositionTarget> mCompositionTarget;
  wil::com_ptr<IDCompositionVisual> mCompositionVisual;
  bool mHaveSystemBackdrop {false};
  bool mIsDisabled {false};

  const wil::unique_hcursor mDefaultCursor {LoadCursorW(nullptr, IDC_ARROW)};
  const wil::unique_hcursor mPointerCursor {LoadCursorW(nullptr, IDC_HAND)};
  const wil::unique_hcursor mTextCursor {LoadCursorW(nullptr, IDC_IBEAM)};
  Cursor mWidgetCursorUnderMouse {};

  wil::unique_hicon mIcon;
  wil::unique_hicon mSmallIcon;

  Win32Window(
    std::unique_ptr<Widgets::Widget> actualRoot,
    Widgets::Widget* immediateRoot,
    HINSTANCE instance,
    int showCommand,
    const Options& options);
  void ResizeSwapchain();
  void AdjustToWindowsTheme();
  void CreateNativeWindow();
  void InitializeDirectComposition();
  [[nodiscard]]
  SIZE GetInitialWindowSize() const;
  void TrackMouseEvent(MouseTrackingArea where);
  void UpdateGeometry(DWORD dpi = 0);
  [[nodiscard]]
  /// Return value indicates if the rect was modified
  bool ApplySizeConstraints(RECT* ncRect) const;

  std::optional<LRESULT> WMSizingProc(WPARAM wParam, LPARAM lParam);
  LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  std::optional<LRESULT>
  TitleBarWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT
  StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  RECT CanvasRectToScreenRect(const Rect& canvasRect) const;

  static Win32Window& Get(HWND hwnd);

  // Windows gives us Client coords and Screen coords; normalize to Screen,
  // *then* figure out that maps to the Canvas.
  [[nodiscard]]
  MouseEvent MakeMouseEventFromClient(WPARAM wParam, LPARAM point);
  [[nodiscard]]
  MouseEvent MakeMouseEventFromScreen(WPARAM wParam, LPARAM lParam);
  MouseEvent MakeMouseEventFromScreen(WPARAM wParam, const POINT& screenPoint);
};
}// namespace FredEmmott::GUI
