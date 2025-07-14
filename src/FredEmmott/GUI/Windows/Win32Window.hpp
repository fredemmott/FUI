// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <Windows.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <wil/com.h>

#include <FredEmmott/GUI/Immediate/Root.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <chrono>
#include <optional>

#include "FredEmmott/GUI/Point.hpp"
#include "FredEmmott/memory.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI {
struct WindowOptions {
  std::string mTitle;

  NativePoint mInitialPosition {CW_USEDEFAULT, CW_USEDEFAULT};

  std::string mClass;
  HWND mParentWindow {nullptr};

  DWORD mWindowStyle {WS_OVERLAPPEDWINDOW};
  DWORD mWindowExStyle {
    WS_EX_APPWINDOW | WS_EX_CLIENTEDGE | WS_EX_NOREDIRECTIONBITMAP};

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
    WinRTMultithreaded,// implies ApartmentThreaded
  };
  enum class COMCleanupMode {
    None,
    Uninitialize,
  };
  enum class DPIMode {
    Uninitialized,
    PerMonitorV2,
  };
  COMMode mCOMMode {COMMode::WinRTMultithreaded};
  COMCleanupMode mCOMCleanupMode {COMCleanupMode::Uninitialize};
  DPIMode mDPIMode {DPIMode::PerMonitorV2};

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

  void SetParent(NativeHandle) final;
  void SetInitialPositionInNativeCoords(const NativePoint& native) final;
  void OffsetPositionToDescendant(Widgets::Widget* child) final;
  void ApplySizeConstraints() override;
  void ResizeToIdeal() override;

  NativePoint CanvasPointToNativePoint(const Point& canvas) const final;

  std::unique_ptr<Window> CreatePopup() const final;

  void WaitForInput() const override;
  void SetIsModal(bool modal);
  void SetResizeMode(ResizeMode horizontal, ResizeMode vertical) override;

 protected:
  static constexpr UINT SwapChainLength = 3;

  Win32Window(HINSTANCE instance, int showCommand, const Options& options);

  void InitializeWindow() final;
  void HideWindow() final;
  void ResizeIfNeeded() final;
  Size GetClientAreaSize() const final;
  float GetDPIScale() const final;
  Color GetClearColor() const final;

  virtual std::unique_ptr<Win32Window>
  CreatePopup(HINSTANCE instance, int showCommand, const Options& options) const
    = 0;
  virtual IUnknown* GetDirectCompositionTargetDevice() const = 0;
  virtual void CreateRenderTargets() = 0;
  virtual void CleanupFrameContexts() = 0;

  auto GetDXGIFactory() const noexcept {
    return mDXGIFactory.get();
  }

  auto GetSwapChain() const noexcept {
    return mSwapChain.get();
  }

  bool IsDisabled() const override {
    return mIsDisabled;
  }

 private:
  HINSTANCE mInstanceHandle {nullptr};
  int mShowCommand {SW_SHOW};
  Options mOptions {};
  static thread_local std::unordered_map<HWND, Win32Window*> gInstances;
  static thread_local Win32Window* gInstanceCreatingWindow;

  HWND mParentHwnd {nullptr};
  wil::unique_hwnd mHwnd;
  std::vector<HWND> mChildren;

  float mDPIScale = {1.0f};
  std::optional<DWORD> mDPI;
  RECT mNCRect {};
  SIZE mClientSize {};
  float mMinimumCanvasWidth {};
  Widgets::Widget* mOffsetToChild {nullptr};
  bool mPendingResize {false};
  bool mTrackingMouseEvents {false};
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

  void ResizeSwapchain();
  void AdjustToWindowsTheme();
  void CreateNativeWindow();
  void InitializeDirectComposition();
  [[nodiscard]]
  SIZE GetInitialWindowSize() const;
  void TrackMouseEvent();
  void SetDPI(WORD newDPI);
  void ApplySizeConstraints(RECT* ncRect) const;

  std::optional<LRESULT> WMSizingProc(WPARAM wParam, LPARAM lParam);
  LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT
  StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
}// namespace FredEmmott::GUI
