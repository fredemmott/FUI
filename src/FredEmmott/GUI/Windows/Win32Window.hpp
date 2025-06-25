// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <Windows.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <wil/com.h>

#include <FredEmmott/GUI/ActivatedFlag.hpp>
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

class Win32Window : public Window {
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

  static unique_ptr<Win32Window>
  CreateAny(HINSTANCE hinstance, int showCommand, const Options& options = {});

  ~Win32Window() override;

  void SetSystemBackdropType(DWM_SYSTEMBACKDROP_TYPE);

  [[nodiscard]]
  NativeHandle GetNativeHandle() const noexcept final {
    return {mHwnd ? mHwnd.get() : nullptr};
  }

  void SetParent(NativeHandle) final;
  void SetInitialPositionInNativeCoords(const NativePoint& native) final;
  void OffsetPositionToDescendant(Widgets::Widget* child) final;

  NativePoint CanvasPointToNativePoint(const Point& canvas) const final;

  std::unique_ptr<Window> CreatePopup() const final;

 protected:
  static constexpr UINT SwapChainLength = 3;

  void InitializeWindow() final;
  void ResizeIfNeeded() final;
  Size GetClientAreaSize() const final;
  float GetDPIScale() const final;
  Color GetClearColor() const final;

  virtual std::unique_ptr<Win32Window>
  CreatePopup(HINSTANCE instance, int showCommand, const Options& options) const
    = 0;
  virtual void InitializeGraphicsAPI() = 0;
  virtual IUnknown* GetDirectCompositionTargetDevice() const = 0;
  virtual void CreateRenderTargets() = 0;
  virtual void CleanupFrameContexts() = 0;

  auto GetDXGIFactory() const noexcept {
    return mDXGIFactory.get();
  }

  auto GetSwapChain() const noexcept {
    return mSwapChain.get();
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
  Widgets::Widget* mOffsetToChild {nullptr};
  ActivatedFlag mPendingResize;
  bool mTrackingMouseEvents = false;
  NativePoint mPosition {};
  int mMinimumWidth {};

  wil::com_ptr<IDXGIFactory4> mDXGIFactory;
  wil::com_ptr<IDXGISwapChain1> mSwapChain;

  wil::com_ptr<IDCompositionDevice> mCompositionDevice;
  wil::com_ptr<IDCompositionTarget> mCompositionTarget;
  wil::com_ptr<IDCompositionVisual> mCompositionVisual;
  bool mHaveSystemBackdrop {false};

  void ResizeSwapchain();
  void AdjustToWindowsTheme();
  void CreateNativeWindow();
  void InitializeDirectComposition();
  [[nodiscard]]
  SIZE CalculateInitialWindowSize() const;
  void TrackMouseEvent();
  void SetDPI(WORD newDPI);

  LRESULT
  WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT
  StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
}// namespace FredEmmott::GUI
