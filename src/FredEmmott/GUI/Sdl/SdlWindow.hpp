// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux `Window` base class: SDL3-backed windowing, event pump, clipboard,
// cursor, interrupt. Abstract — rendering is provided by a subclass
// (currently SdlSkiaVulkanWindow, Skia Ganesh on Vulkan), which fills
// in the render-side pure virtuals: InitializeGraphicsAPI, GetFramePainter,
// ResizeBackend, CreatePopup.
#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

#include <FredEmmott/GUI/StylePropertyTypes.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>

// Forward-declare SDL types at the global namespace so the header doesn't
// leak <SDL3/SDL.h> into the rest of FUI. Must stay at global scope —
// writing `union SDL_Event` inside FredEmmott::GUI creates a new incomplete
// type in that namespace instead of finding the SDL one.
struct SDL_Window;
struct SDL_Cursor;
union SDL_Event;

namespace FredEmmott::GUI {

class SdlWindow : public Window {
 public:
  struct Options {
    std::string mTitle = "FUI";
    Size mInitialSize {960, 640};
    ResizeMode mHorizontalResizeMode {ResizeMode::Allow};
    ResizeMode mVerticalResizeMode {ResizeMode::Allow};
  };

  explicit SdlWindow(Options options);
  ~SdlWindow() override;

  // -- Window overrides ------------------------------------------------------

  void SetParent(NativeHandle) override;
  void SetTitle(std::string_view) override;
  [[nodiscard]] bool SetSubtitle(std::string_view) override;
  [[nodiscard]] NativeHandle GetNativeHandle() const noexcept override;
  void SetInitialPositionInNativeCoords(const NativePoint& native) override;
  void OffsetPositionToDescendant(Widgets::Widget* child) override;
  void ResizeToIdeal() override;
  [[nodiscard]] bool IsDisabled() const override;
  void SetResizeMode(ResizeMode horizontal, ResizeMode vertical) override;
  [[nodiscard]] NativePoint CanvasPointToNativePoint(
    const Point& canvas) const override;
  [[nodiscard]] Point NativePointToCanvasPoint(
    const NativePoint& native) const override;

  void InterruptWaitFrame() override;

  [[nodiscard]] std::optional<std::string> GetClipboardText() const override;
  void SetClipboardText(std::string_view) const override;

  [[nodiscard]] bool IsPopup() const noexcept override;
  void SetIsToolTip() override;

 protected:
  // Subclasses (SdlSkiaVulkanWindow) override to add flags like
  // SDL_WINDOW_VULKAN. Base returns resizable + high-DPI flags.
  // Return type is uint64_t to avoid leaking SDL_WindowFlags into the
  // header — the .cpp casts it back to SDL_WindowFlags on use.
  virtual uint64_t GetSDLWindowFlags() const;

  void SetBackdrop(const WindowBackdrop&) override;
  void ProcessNativeEvents() override;
  void InitializeWindow() override;
  void HideWindow() override;
  void ResizeIfNeeded() final;
  [[nodiscard]] Size GetCanvasSize() const override;
  [[nodiscard]] float GetDPIScale() const override;
  [[nodiscard]] Color GetClearColor() const override;
  void WaitFrameImpl(
    std::span<const NativeWaitable> waitables,
    std::chrono::steady_clock::time_point until) const override;

  // Backend hook called by ResizeIfNeeded after popup auto-fit, to let
  // the renderer recreate its swapchain / surfaces for the new SDL window
  // size. Pure virtual so every backend explicitly opts in.
  virtual void ResizeBackend() = 0;

 private:
  // Delegating ctor owns root widgets like Win32Window does, so the Window
  // base has valid non-null roots by the time Immediate::Root constructs.
  SdlWindow(
    std::unique_ptr<Widgets::Widget> actualRoot,
    Widgets::Widget* immediateRoot,
    Options options);

  std::unique_ptr<Widgets::Widget> mActualRoot;
  Widgets::Widget* mImmediateRoot {nullptr};
  Options mOptions;
  SDL_Window* mSDLWindow {nullptr};
  std::atomic<bool> mNeedsResize {false};

  // Popup state. mPopupParent is set by SetParent before InitializeWindow
  // runs; if non-null, InitializeWindow uses SDL_CreatePopupWindow with
  // SDL_WINDOW_POPUP_MENU (or SDL_WINDOW_TOOLTIP if SetIsToolTip was
  // called) instead of SDL_CreateWindow. The offset is parent-relative
  // window coords (SdlWindow's CanvasPointToNativePoint hands those back
  // unchanged), which is exactly what SDL_CreatePopupWindow wants.
  SDL_Window* mPopupParent {nullptr};
  int mPopupOffsetX {0};
  int mPopupOffsetY {0};
  bool mIsTooltip {false};

  // Hover detection. SDL3 doesn't fire a hover event the way Win32's
  // WM_MOUSEHOVER does; we time it ourselves. After kHoverDelay of mouse
  // idleness inside the window, dispatch a single MouseEvent::HoverEvent
  // at the last position. Reset on every motion / button / leave so the
  // next idle period queues another.
  std::chrono::steady_clock::time_point mLastMouseMoveTime {};
  Point mLastMousePos {-1, -1};
  MouseButtons mLastMouseButtons {};
  bool mHoverPending {false};
  bool mMouseInside {false};

  // SDL3's user-event type we push from InterruptWaitFrame to break
  // SDL_WaitEventTimeout in the other thread / deeper call stack.
  uint32_t mInterruptEventType {};

  void DispatchSDLEvent(const ::SDL_Event&);
  void DispatchKeyEvent(const ::SDL_Event&, bool pressed);
  void DispatchMouseMotion(const ::SDL_Event&);
  void DispatchMouseButton(const ::SDL_Event&, bool pressed);
  void DispatchMouseWheel(const ::SDL_Event&);
  void DispatchPenMotion(const ::SDL_Event&);
  void DispatchPenTouch(const ::SDL_Event&, bool pressed);
  void DispatchPenButton(const ::SDL_Event&, bool pressed);
  void HandlePenAxis(const ::SDL_Event&);
  void HandlePenProximity(const ::SDL_Event&, bool entered);

  // Latest known axis state per SDL_PenID. SDL3 sends pressure/tilt/etc. as
  // separate SDL_EVENT_PEN_AXIS events that arrive *between* MOTION/TOUCH
  // events; we cache them here so each dispatched MouseEvent carries the
  // current state. Cleared on SDL_EVENT_PEN_PROXIMITY_OUT.
  std::unordered_map<uint32_t, MouseEvent::PenAxes> mPenAxes;

  // Cursor state. Lazily-created SDL system cursors keyed by Cursor enum
  // (size = number of enumerators). mCurrentCursor avoids redundant
  // SDL_SetCursor calls on every mouse-move event.
  Cursor mCurrentCursor {Cursor::Default};
  std::array<SDL_Cursor*, 3> mCursors {};
  void ApplyCursor(Cursor desired);

  void MaybeDispatchHover();
};

}// namespace FredEmmott::GUI
