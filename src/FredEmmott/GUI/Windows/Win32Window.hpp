// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <Windows.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <wil/com.h>

#include <FredEmmott/GUI/ActivatedFlag.hpp>
#include <FredEmmott/GUI/Immediate/Root.hpp>
#include <chrono>
#include <expected>
#include <optional>

namespace FredEmmott::GUI::Widgets {
class Widget;
}

#include "FredEmmott/GUI/Point.hpp"

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
};

class Win32Window {
 public:
  using Options = WindowOptions;

  Win32Window() = delete;
  Win32Window(const Win32Window&) = delete;
  Win32Window(Win32Window&&) = delete;
  Win32Window& operator=(const Win32Window&) = delete;
  Win32Window& operator=(Win32Window&&) = delete;

  explicit Win32Window(
    HINSTANCE instance,
    int showCommand,
    const Options& options = {});

  virtual ~Win32Window();

  void SetSystemBackdropType(DWM_SYSTEMBACKDROP_TYPE);

  [[nodiscard]]
  HWND GetNativeHandle() const noexcept {
    return mHwnd ? mHwnd.get() : nullptr;
  }

  void SetParent(HWND);

  void SetInitialPositionInNativeCoords(const NativePoint& native);

  NativePoint CanvasPointToNativePoint(const Point& canvas) const;

  [[nodiscard]]
  std::expected<void, int> BeginFrame();
  void WaitFrame(unsigned int minFPS = 0, unsigned int maxFPS = 60) const;
  void EndFrame();

  FrameRateRequirement GetFrameRateRequirement() const;

  void OffsetPositionToDescendant(Widgets::Widget* child);

 protected:
  class BasicFramePainter {
   public:
    virtual ~BasicFramePainter() = default;

    virtual Renderer* GetRenderer() noexcept = 0;
  };
  static constexpr UINT SwapChainLength = 3;

  virtual std::unique_ptr<BasicFramePainter> GetFramePainter(
    uint8_t mFrameIndex)
    = 0;
  virtual void InitializeGraphicsAPI() = 0;
  virtual IUnknown* GetDirectCompositionTargetDevice() const = 0;
  virtual void CreateRenderTargets() = 0;
  virtual void CleanupFrameContexts() = 0;

 private:
  HINSTANCE mInstanceHandle {nullptr};
  int mShowCommand {SW_SHOW};
  Options mOptions {};
  static thread_local std::unordered_map<HWND, Win32Window*> gInstances;
  static thread_local Win32Window* gInstanceCreatingWindow;

  std::optional<int> mExitCode;

  HWND mParentHwnd {nullptr};
  wil::unique_hwnd mHwnd;
  std::vector<HWND> mChildren;

  float mDPIScale = {1.0f};
  std::optional<DWORD> mDPI;
  RECT mNCRect {};
  SIZE mClientSize {};
  Widgets::Widget* mOffsetToChild {nullptr};
  ActivatedFlag mPendingResize;
  bool mTrackingMouseEvents = false;
  NativePoint mPosition {};
  int mMinimumWidth {};

  Immediate::Root mFUIRoot;

  wil::com_ptr<IDCompositionDevice> mCompositionDevice;
  wil::com_ptr<IDCompositionTarget> mCompositionTarget;
  wil::com_ptr<IDCompositionVisual> mCompositionVisual;
  bool mHaveSystemBackdrop {false};

  std::chrono::steady_clock::time_point mBeginFrameTime;
  uint8_t mFrameIndex {};// Used to index into mFrames; reset when buffer reset

  void ResizeIfNeeded();
  void ResizeSwapchain();
  void AdjustToWindowsTheme();
  void InitializeWindow();
  void CreateNativeWindow();
  void InitializeDirectComposition();
  [[nodiscard]]
  SIZE CalculateInitialWindowSize() const;
  void Paint();
  void TrackMouseEvent();
  void SetDPI(WORD newDPI);

  LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT
  StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
}// namespace FredEmmott::GUI
