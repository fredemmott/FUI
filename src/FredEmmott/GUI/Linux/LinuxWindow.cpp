// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux `Window` base class — see LinuxWindow.hpp for scope.

#include "LinuxWindow.hpp"
#include "TooltipPassthrough.hpp"

#include <SDL3/SDL.h>
#include <Yoga.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/StyleClass.hpp>
#include <FredEmmott/GUI/StylePropertyTypes.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/events/KeyCode.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <FredEmmott/GUI/events/MouseButton.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>
#include <FredEmmott/GUI/events/TextInputEvent.hpp>

namespace FredEmmott::GUI {

namespace {

// Allocated once per process on first LinuxWindow construction.
uint32_t gInterruptEventType = 0;

constexpr LiteralStyleClass ActualRootStyleClass {"LinuxWindow/Root"};
constexpr LiteralStyleClass ImmediateRootStyleClass {
  "LinuxWindow/ImmediateRoot"};

const auto& ActualRootStyles() {
  static const ImmutableStyle ret {
    Style().FlexDirection(FlexDirection::Column).FlexGrow(1),
  };
  return ret;
}

const auto& ImmediateRootStyles() {
  static const ImmutableStyle ret {
    ActualRootStyles() + Style().FlexShrink(1).FlexGrow(1),
  };
  return ret;
}

// Very small SDL_Keycode / SDL_Scancode → FUI KeyCode mapping. FUI's KeyCode
// mirrors Win32 VK_* values; we only map the keys its widgets currently use
// (see events/KeyCode.hpp). Anything we don't recognise is dropped — its
// text-input side is handled separately via SDL_EVENT_TEXT_INPUT.
std::optional<KeyCode> MapKeycode(SDL_Keycode sym) {
  using enum KeyCode;
  switch (sym) {
    case SDLK_TAB:
      return Key_Tab;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      return Key_Return;
    case SDLK_BACKSPACE:
      return Key_Backspace;
    case SDLK_ESCAPE:
      return Key_Escape;
    case SDLK_SPACE:
      return Key_Space;
    case SDLK_END:
      return Key_End;
    case SDLK_HOME:
      return Key_Home;
    case SDLK_LEFT:
      return Key_LeftArrow;
    case SDLK_UP:
      return Key_UpArrow;
    case SDLK_RIGHT:
      return Key_RightArrow;
    case SDLK_DOWN:
      return Key_DownArrow;
    case SDLK_INSERT:
      return Key_Insert;
    case SDLK_DELETE:
      return Key_Delete;
    default:
      break;
  }
  if (sym >= SDLK_A && sym <= SDLK_Z) {
    // FUI Key_A == 0x41 == 'A'; SDLK_A is also 'a' (0x61) in SDL3.
    const auto offset = static_cast<int>(sym) - static_cast<int>(SDLK_A);
    return static_cast<KeyCode>(static_cast<int>(Key_A) + offset);
  }
  return std::nullopt;
}

KeyModifier MapKeymod(SDL_Keymod mod) {
  auto ret = KeyModifier::Modifier_None;
  if (mod & SDL_KMOD_SHIFT) {
    ret |= KeyModifier::Modifier_Shift;
  }
  if (mod & SDL_KMOD_CTRL) {
    ret |= KeyModifier::Modifier_Control;
  }
  if (mod & SDL_KMOD_ALT) {
    ret |= KeyModifier::Modifier_Alt;
  }
  return ret;
}

MouseButton MapMouseButton(uint8_t sdlButton) {
  switch (sdlButton) {
    case SDL_BUTTON_LEFT:
      return MouseButton::Left;
    case SDL_BUTTON_RIGHT:
      return MouseButton::Right;
    case SDL_BUTTON_MIDDLE:
      return MouseButton::Middle;
    case SDL_BUTTON_X1:
      return MouseButton::X1;
    case SDL_BUTTON_X2:
      return MouseButton::X2;
  }
  return MouseButton::None;
}

MouseButtons MapMouseButtonMask(uint32_t sdlMask) {
  auto ret = MouseButtons {};
  if (sdlMask & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
    ret |= MouseButton::Left;
  }
  if (sdlMask & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
    ret |= MouseButton::Right;
  }
  if (sdlMask & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) {
    ret |= MouseButton::Middle;
  }
  if (sdlMask & SDL_BUTTON_MASK(SDL_BUTTON_X1)) {
    ret |= MouseButton::X1;
  }
  if (sdlMask & SDL_BUTTON_MASK(SDL_BUTTON_X2)) {
    ret |= MouseButton::X2;
  }
  return ret;
}

}// namespace

LinuxWindow::LinuxWindow(Options options)
  : LinuxWindow(
      std::make_unique<Widgets::Widget>(
        this,
        ActualRootStyleClass,
        ActualRootStyles()),
      new Widgets::Widget(
        this,
        ImmediateRootStyleClass,
        ImmediateRootStyles()),
      std::move(options)) {
}

LinuxWindow::LinuxWindow(
  std::unique_ptr<Widgets::Widget> actualRoot,
  Widgets::Widget* immediateRoot,
  Options options)
  : Window(actualRoot.get(), immediateRoot, /*swapChainLength=*/2),
    mActualRoot(std::move(actualRoot)),
    mImmediateRoot(immediateRoot),
    mOptions(std::move(options)) {
  mActualRoot->SetStructuralChildren({mImmediateRoot});

  // SDL_InitSubSystem is refcounted — safe to call multiple times; matching
  // QuitSubSystem calls in each dtor decrement the count.
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    throw std::runtime_error(
      std::string {"SDL_InitSubSystem(VIDEO) failed: "} + SDL_GetError());
  }
  // Stop SDL3 from synthesising mouse events for pen input — we handle the
  // pen event stream ourselves below and re-emit MouseEvents with mPenAxes
  // populated. Without this hint the same pen action would dispatch twice.
  SDL_SetHint(SDL_HINT_PEN_MOUSE_EVENTS, "0");
  if (gInterruptEventType == 0) {
    gInterruptEventType = SDL_RegisterEvents(1);
  }
  mInterruptEventType = gInterruptEventType;
}

LinuxWindow::~LinuxWindow() {
  for (auto*& cursor : mCursors) {
    if (cursor) {
      SDL_DestroyCursor(cursor);
      cursor = nullptr;
    }
  }
  if (mSDLWindow) {
    SDL_DestroyWindow(mSDLWindow);
    mSDLWindow = nullptr;
  }
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

// -- InitializeWindow: actually opens the SDL3 window ----------------------

uint64_t LinuxWindow::GetSDLWindowFlags() const {
  return SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
}

void LinuxWindow::InitializeWindow() {
  if (mSDLWindow) {
    return;
  }
  if (mPopupParent) {
    // Popup-mode: SDL_CreatePopupWindow takes parent-relative coords and
    // requires SDL_WINDOW_TOOLTIP or SDL_WINDOW_POPUP_MENU. Tooltips don't
    // grab focus; popup menus do (SDL3 handles the dismissal — clicking
    // outside auto-hides the popup window).
    const auto popupFlag
      = mIsTooltip ? SDL_WINDOW_TOOLTIP : SDL_WINDOW_POPUP_MENU;
    mSDLWindow = SDL_CreatePopupWindow(
      mPopupParent,
      mPopupOffsetX,
      mPopupOffsetY,
      static_cast<int>(mOptions.mInitialSize.mWidth),
      static_cast<int>(mOptions.mInitialSize.mHeight),
      static_cast<SDL_WindowFlags>(this->GetSDLWindowFlags() | popupFlag));
    if (!mSDLWindow) {
      std::fprintf(
        stderr, "SDL_CreatePopupWindow failed: %s\n", SDL_GetError());
      return;
    }
    if (mIsTooltip) {
      // Tooltip popups must be mouse-passthrough so the parent widget keeps
      // the cursor during a drag. Win32 uses WS_EX_TRANSPARENT; on Linux we
      // clear the surface's input region directly (Wayland or X11 path).
      Linux::MakePopupInputPassthrough(mSDLWindow);
    }
    return;
  }
  mSDLWindow = SDL_CreateWindow(
    mOptions.mTitle.c_str(),
    static_cast<int>(mOptions.mInitialSize.mWidth),
    static_cast<int>(mOptions.mInitialSize.mHeight),
    static_cast<SDL_WindowFlags>(this->GetSDLWindowFlags()));
  if (!mSDLWindow) {
    std::fprintf(
      stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    return;
  }
  this->SetResizeMode(
    mOptions.mHorizontalResizeMode, mOptions.mVerticalResizeMode);
}

void LinuxWindow::HideWindow() {
  if (mSDLWindow) {
    SDL_HideWindow(mSDLWindow);
  }
}

// -- Event pump ------------------------------------------------------------

// Pull windowID off whichever sub-struct of SDL_Event holds it, falling
// back to 0 (SDL_WINDOW_ID_INVALID) for events that aren't bound to a
// specific window (SDL_EVENT_QUIT, our interrupt user-event, etc.). Those
// pass through regardless.
static SDL_WindowID EventTargetWindowID(const SDL_Event& e) {
  switch (e.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
      return e.key.windowID;
    case SDL_EVENT_TEXT_INPUT:
      return e.text.windowID;
    case SDL_EVENT_TEXT_EDITING:
      return e.edit.windowID;
    case SDL_EVENT_MOUSE_MOTION:
      return e.motion.windowID;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
      return e.button.windowID;
    case SDL_EVENT_MOUSE_WHEEL:
      return e.wheel.windowID;
    case SDL_EVENT_PEN_PROXIMITY_IN:
    case SDL_EVENT_PEN_PROXIMITY_OUT:
      return e.pproximity.windowID;
    case SDL_EVENT_PEN_AXIS:
      return e.paxis.windowID;
    case SDL_EVENT_PEN_MOTION:
      return e.pmotion.windowID;
    case SDL_EVENT_PEN_DOWN:
    case SDL_EVENT_PEN_UP:
      return e.ptouch.windowID;
    case SDL_EVENT_PEN_BUTTON_DOWN:
    case SDL_EVENT_PEN_BUTTON_UP:
      return e.pbutton.windowID;
    default:
      // SDL_EVENT_WINDOW_* events all use e.window.windowID.
      if (
        e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST) {
        return e.window.windowID;
      }
      return 0;
  }
}

void LinuxWindow::ProcessNativeEvents() {
  if (!mSDLWindow) {
    return;
  }
  const auto myID = SDL_GetWindowID(mSDLWindow);

  // FUI runs each Window's frame loop independently, including popup
  // children that BeginFrame inside the parent's appTick. SDL_PollEvent
  // drains globally, so without per-window filtering the parent's
  // ProcessNativeEvents would steal events meant for an open popup —
  // including clicks on popup-menu items, which is what closes the popup.
  // Filter by windowID; requeue events that aren't ours so the other
  // window's ProcessNativeEvents picks them up.
  std::vector<SDL_Event> requeue;
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    const auto target = EventTargetWindowID(e);
    if (target == 0 || target == myID) {
      this->DispatchSDLEvent(e);
    } else {
      requeue.push_back(e);
    }
  }
  for (const auto& re : requeue) {
    SDL_PushEvent(const_cast<SDL_Event*>(&re));
  }

  // Hover dispatch happens after event drain — once nothing fresh is
  // disturbing the mouse state and the idle threshold has elapsed,
  // synthesize the HoverEvent for the widget tree.
  this->MaybeDispatchHover();
}

void LinuxWindow::DispatchSDLEvent(const SDL_Event& e) {
  if (e.type == mInterruptEventType) {
    return;
  }
  switch (e.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      this->RequestStop(EXIT_SUCCESS);
      return;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
      // Click-outside dismissal for popup menus. Wayland's xdg_popup
      // already loses focus when the user clicks elsewhere; SDL3 surfaces
      // that as FOCUS_LOST. Tooltips don't get focus to begin with so this
      // path is a no-op for them — they're dismissed by the showing
      // widget noticing the mouse left its anchor.
      // Hide the SDL window now too: Window::EndFrame's HideWindow path
      // only runs if EndFrame is reached, which it isn't on the
      // BeginFrame-failure cleanup branch in BeginBasicPopupWindow.
      if (mPopupParent && !mIsTooltip) {
        this->RequestStop(EXIT_SUCCESS);
        SDL_HideWindow(mSDLWindow);
      }
      return;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      mNeedsResize.store(true, std::memory_order_release);
      return;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
      mMouseInside = true;
      mLastMouseMoveTime = std::chrono::steady_clock::now();
      mHoverPending = true;
      return;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
      mMouseInside = false;
      mHoverPending = false;
      return;
    case SDL_EVENT_KEY_DOWN:
      this->DispatchKeyEvent(e, /*pressed=*/true);
      return;
    case SDL_EVENT_KEY_UP:
      this->DispatchKeyEvent(e, /*pressed=*/false);
      return;
    case SDL_EVENT_TEXT_INPUT: {
      const TextInputEvent te {e.text.text ? e.text.text : ""};
      this->DispatchEvent(te);
      return;
    }
    case SDL_EVENT_MOUSE_MOTION:
      this->DispatchMouseMotion(e);
      return;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      this->DispatchMouseButton(e, /*pressed=*/true);
      return;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      this->DispatchMouseButton(e, /*pressed=*/false);
      return;
    case SDL_EVENT_MOUSE_WHEEL:
      this->DispatchMouseWheel(e);
      return;
    case SDL_EVENT_PEN_PROXIMITY_IN:
      this->HandlePenProximity(e, /*entered=*/true);
      return;
    case SDL_EVENT_PEN_PROXIMITY_OUT:
      this->HandlePenProximity(e, /*entered=*/false);
      return;
    case SDL_EVENT_PEN_AXIS:
      this->HandlePenAxis(e);
      return;
    case SDL_EVENT_PEN_MOTION:
      this->DispatchPenMotion(e);
      return;
    case SDL_EVENT_PEN_DOWN:
      this->DispatchPenTouch(e, /*pressed=*/true);
      return;
    case SDL_EVENT_PEN_UP:
      this->DispatchPenTouch(e, /*pressed=*/false);
      return;
    case SDL_EVENT_PEN_BUTTON_DOWN:
      this->DispatchPenButton(e, /*pressed=*/true);
      return;
    case SDL_EVENT_PEN_BUTTON_UP:
      this->DispatchPenButton(e, /*pressed=*/false);
      return;
    default:
      break;
  }
}

void LinuxWindow::DispatchKeyEvent(const SDL_Event& e, bool pressed) {
  const auto keyCode = MapKeycode(e.key.key);
  if (!keyCode) {
    return;
  }
  const auto mods = MapKeymod(static_cast<SDL_Keymod>(e.key.mod));
  if (pressed) {
    const KeyPressEvent ev {*keyCode, mods};
    this->DispatchEvent(ev);
  } else {
    const KeyReleaseEvent ev {*keyCode, mods};
    this->DispatchEvent(ev);
  }
}

void LinuxWindow::DispatchMouseMotion(const SDL_Event& e) {
  MouseEvent me;
  me.mWindowPoint = Point {e.motion.x, e.motion.y};
  me.mButtons = MapMouseButtonMask(e.motion.state);
  me.mDetail = MouseEvent::MoveEvent {};
  const auto* receiver = this->DispatchEvent(me);
  this->ApplyCursor(
    receiver ? receiver->GetComputedStyle().Cursor().value_or(Cursor::Default)
             : Cursor::Default);

  // Arm hover detection: every motion resets the timer.
  mLastMouseMoveTime = std::chrono::steady_clock::now();
  mLastMousePos = me.mWindowPoint;
  mLastMouseButtons = me.mButtons;
  mMouseInside = true;
  mHoverPending = true;
}

void LinuxWindow::DispatchMouseButton(const SDL_Event& e, bool pressed) {
  const auto btn = MapMouseButton(e.button.button);
  if (btn == MouseButton::None) {
    return;
  }
  MouseEvent me;
  me.mWindowPoint = Point {e.button.x, e.button.y};
  // SDL_MouseButtonEvent doesn't carry the full button mask on Linux;
  // re-query it for OR'ing into FUI's event.
  float _x {}, _y {};
  me.mButtons = MapMouseButtonMask(SDL_GetMouseState(&_x, &_y));
  if (pressed) {
    // SDL3's clicks counter is OS-tuned for double-click time and pixel
    // drift; 1 = single, 2 = second of a double-click, 3 = third of a
    // triple-click. Synthesised events report 0, so clamp to 1.
    const auto clicks = static_cast<std::uint8_t>(
      e.button.clicks > 0 ? e.button.clicks : 1);
    me.mDetail = MouseEvent::ButtonPressEvent {btn, clicks};
  } else {
    me.mDetail = MouseEvent::ButtonReleaseEvent {btn};
  }
  this->DispatchEvent(me);

  // A click is "interaction"; reset the hover idle timer.
  mLastMouseMoveTime = std::chrono::steady_clock::now();
  mLastMousePos = me.mWindowPoint;
  mLastMouseButtons = me.mButtons;
  mHoverPending = true;
}

// -- Hover -----------------------------------------------------------------

namespace {
// Default WinUI3 / Win32 mouse-hover delay. Win32's
// SystemParametersInfo(SPI_GETMOUSEHOVERTIME) reports 400ms by default;
// match that.
constexpr std::chrono::milliseconds kHoverDelay {400};
}// namespace

void LinuxWindow::MaybeDispatchHover() {
  if (!mHoverPending || !mMouseInside || mLastMousePos.mX < 0) {
    return;
  }
  // Don't fire hover while a button is held — typically a drag.
  if (mLastMouseButtons != MouseButtons {}) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  if (now - mLastMouseMoveTime < kHoverDelay) {
    return;
  }

  MouseEvent me;
  me.mWindowPoint = mLastMousePos;
  me.mButtons = mLastMouseButtons;
  me.mDetail = MouseEvent::HoverEvent {};
  this->DispatchEvent(me);
  mHoverPending = false;
}

// -- Pen events ------------------------------------------------------------
//
// SDL3's pen stream is decoupled from the mouse stream (we disabled the
// synthetic mouse path via SDL_HINT_PEN_MOUSE_EVENTS in the ctor). Axis
// values arrive in standalone SDL_EVENT_PEN_AXIS events between
// MOTION/TOUCH events, so we cache the latest per-pen state in mPenAxes
// and copy it into each dispatched MouseEvent.

namespace {

MouseButton MapPenBarrelButton(uint8_t sdlButtonIndex) {
  // Wacom / Linux convention: lower barrel button = right click,
  // upper barrel button = middle click. Higher indices have no widely
  // accepted mapping; drop them.
  switch (sdlButtonIndex) {
    case 1:
      return MouseButton::Right;
    case 2:
      return MouseButton::Middle;
    default:
      return MouseButton::None;
  }
}

}// namespace

void LinuxWindow::HandlePenProximity(const SDL_Event& e, bool entered) {
  const auto id = static_cast<uint32_t>(e.pproximity.which);
  if (entered) {
    mPenAxes[id] = MouseEvent::PenAxes {};
  } else {
    mPenAxes.erase(id);
  }
}

void LinuxWindow::HandlePenAxis(const SDL_Event& e) {
  auto& axes = mPenAxes[static_cast<uint32_t>(e.paxis.which)];
  axes.mEraser = (e.paxis.pen_state & SDL_PEN_INPUT_ERASER_TIP) != 0;
  switch (e.paxis.axis) {
    case SDL_PEN_AXIS_PRESSURE:
      axes.mPressure = e.paxis.value;
      return;
    case SDL_PEN_AXIS_XTILT:
      axes.mTiltX = e.paxis.value;
      return;
    case SDL_PEN_AXIS_YTILT:
      axes.mTiltY = e.paxis.value;
      return;
    case SDL_PEN_AXIS_DISTANCE:
      axes.mDistance = e.paxis.value;
      return;
    case SDL_PEN_AXIS_ROTATION:
      axes.mRotation = e.paxis.value;
      return;
    default:
      // Slider / tangential pressure / future axes — not surfaced via
      // PenAxes yet. Drop silently rather than fail.
      return;
  }
}

void LinuxWindow::DispatchPenMotion(const SDL_Event& e) {
  auto& axes = mPenAxes[static_cast<uint32_t>(e.pmotion.which)];
  axes.mEraser = (e.pmotion.pen_state & SDL_PEN_INPUT_ERASER_TIP) != 0;
  MouseEvent me;
  me.mWindowPoint = Point {e.pmotion.x, e.pmotion.y};
  // Reflect "tip down" as a held Left button so existing widgets see drag
  // as a left-button drag.
  if (e.pmotion.pen_state & SDL_PEN_INPUT_DOWN) {
    me.mButtons |= MouseButton::Left;
  }
  me.mPenAxes = axes;
  me.mDetail = MouseEvent::MoveEvent {};
  const auto* receiver = this->DispatchEvent(me);
  this->ApplyCursor(
    receiver ? receiver->GetComputedStyle().Cursor().value_or(Cursor::Default)
             : Cursor::Default);
}

void LinuxWindow::DispatchPenTouch(const SDL_Event& e, bool pressed) {
  auto& axes = mPenAxes[static_cast<uint32_t>(e.ptouch.which)];
  axes.mEraser = e.ptouch.eraser;
  MouseEvent me;
  me.mWindowPoint = Point {e.ptouch.x, e.ptouch.y};
  if (e.ptouch.pen_state & SDL_PEN_INPUT_DOWN) {
    me.mButtons |= MouseButton::Left;
  }
  me.mPenAxes = axes;
  if (pressed) {
    me.mDetail = MouseEvent::ButtonPressEvent {MouseButton::Left};
  } else {
    me.mDetail = MouseEvent::ButtonReleaseEvent {MouseButton::Left};
  }
  this->DispatchEvent(me);
}

void LinuxWindow::DispatchPenButton(const SDL_Event& e, bool pressed) {
  const auto btn = MapPenBarrelButton(e.pbutton.button);
  if (btn == MouseButton::None) {
    return;
  }
  auto& axes = mPenAxes[static_cast<uint32_t>(e.pbutton.which)];
  axes.mEraser = (e.pbutton.pen_state & SDL_PEN_INPUT_ERASER_TIP) != 0;
  MouseEvent me;
  me.mWindowPoint = Point {e.pbutton.x, e.pbutton.y};
  if (e.pbutton.pen_state & SDL_PEN_INPUT_DOWN) {
    me.mButtons |= MouseButton::Left;
  }
  me.mButtons |= btn;
  me.mPenAxes = axes;
  if (pressed) {
    me.mDetail = MouseEvent::ButtonPressEvent {btn};
  } else {
    me.mDetail = MouseEvent::ButtonReleaseEvent {btn};
  }
  this->DispatchEvent(me);
}

void LinuxWindow::DispatchMouseWheel(const SDL_Event& e) {
  MouseEvent me;
  me.mWindowPoint = Point {e.wheel.mouse_x, e.wheel.mouse_y};
  float _x {}, _y {};
  me.mButtons = MapMouseButtonMask(SDL_GetMouseState(&_x, &_y));
  // SDL3 reports positive Y for "scroll up" — matches FUI convention.
  if (e.wheel.y != 0) {
    me.mDetail = MouseEvent::VerticalWheelEvent {e.wheel.y};
    this->DispatchEvent(me);
  }
  if (e.wheel.x != 0) {
    me.mDetail = MouseEvent::HorizontalWheelEvent {e.wheel.x};
    this->DispatchEvent(me);
  }
}

// -- Wait / interrupt ------------------------------------------------------

void LinuxWindow::InterruptWaitFrame() {
  if (mInterruptEventType == 0) {
    return;
  }
  SDL_Event e {};
  e.type = mInterruptEventType;
  SDL_PushEvent(&e);
}

void LinuxWindow::WaitFrameImpl(
  std::span<const NativeWaitable> waitables,
  std::chrono::steady_clock::time_point until) const {
  // TODO: waitables (FDs) are ignored. SDL_WaitEventTimeout blocks until
  // an event or the timeout. A proper self-pipe + poll() that also handles
  // the NativeWaitable FDs passed in is a follow-up.
  (void)waitables;
  using namespace std::chrono;
  const auto now = steady_clock::now();
  if (until <= now) {
    return;
  }
  // Cap the wait at the next hover-fire deadline so an idle mouse still
  // gets its HoverEvent. Without this we'd block until the next user input
  // and never fire hover when the user just rests on a tooltip target.
  auto deadline = until;
  if (mHoverPending && mMouseInside && mLastMouseButtons == MouseButtons {}) {
    const auto hoverWake = mLastMouseMoveTime + kHoverDelay;
    if (hoverWake < deadline) {
      deadline = hoverWake;
    }
  }
  if (deadline <= now) {
    return;
  }
  const auto remaining = duration_cast<milliseconds>(deadline - now).count();
  const int32_t timeout
    = (deadline == steady_clock::time_point::max() || remaining > INT32_MAX)
    ? -1
    : static_cast<int32_t>(std::max<int64_t>(0, remaining));
  SDL_WaitEventTimeout(nullptr, timeout);
}

// -- Cursor ----------------------------------------------------------------

namespace {

SDL_SystemCursor MapCursor(Cursor c) {
  switch (c) {
    case Cursor::Default:
      return SDL_SYSTEM_CURSOR_DEFAULT;
    case Cursor::Pointer:
      return SDL_SYSTEM_CURSOR_POINTER;
    case Cursor::Text:
      return SDL_SYSTEM_CURSOR_TEXT;
  }
  return SDL_SYSTEM_CURSOR_DEFAULT;
}

}// namespace

void LinuxWindow::ApplyCursor(Cursor desired) {
  if (desired == mCurrentCursor && mCursors[static_cast<size_t>(desired)]) {
    return;
  }
  const auto idx = static_cast<size_t>(desired);
  if (!mCursors[idx]) {
    mCursors[idx] = SDL_CreateSystemCursor(MapCursor(desired));
    if (!mCursors[idx]) {
      // Cursor creation failure is rare (out-of-mem) and non-fatal — keep the
      // previously-applied cursor and don't update mCurrentCursor.
      return;
    }
  }
  SDL_SetCursor(mCursors[idx]);
  mCurrentCursor = desired;
}

// -- Clipboard -------------------------------------------------------------

std::optional<std::string> LinuxWindow::GetClipboardText() const {
  if (!SDL_HasClipboardText()) {
    return std::nullopt;
  }
  char* const owned = SDL_GetClipboardText();
  if (!owned) {
    return std::nullopt;
  }
  std::string ret {owned};
  SDL_free(owned);
  return ret;
}

void LinuxWindow::SetClipboardText(std::string_view text) const {
  const std::string nul {text};// SDL needs null-terminated
  SDL_SetClipboardText(nul.c_str());
}

// -- Window-level getters / setters ----------------------------------------

void LinuxWindow::SetTitle(std::string_view text) {
  const std::string nul {text};
  mOptions.mTitle = nul;
  if (mSDLWindow) {
    SDL_SetWindowTitle(mSDLWindow, nul.c_str());
  }
}

bool LinuxWindow::SetSubtitle(std::string_view) {
  return false;
}

Window::NativeHandle LinuxWindow::GetNativeHandle() const noexcept {
  return NativeHandle {mSDLWindow};
}

void LinuxWindow::SetInitialPositionInNativeCoords(const NativePoint& native) {
  if (mPopupParent && !mSDLWindow) {
    // Popup-mode pre-creation: stash for SDL_CreatePopupWindow's
    // offset_x/offset_y. LinuxWindow's CanvasPointToNativePoint returns
    // window-local coords (just rounded), which is exactly what
    // SDL_CreatePopupWindow expects ("relative to the origin of the
    // parent").
    mPopupOffsetX = native.mX;
    mPopupOffsetY = native.mY;
    return;
  }
  if (mSDLWindow) {
    SDL_SetWindowPosition(mSDLWindow, native.mX, native.mY);
  }
}

void LinuxWindow::SetResizeMode(ResizeMode horizontal, ResizeMode vertical) {
  if (!mSDLWindow) {
    return;
  }
  const bool anyAllow = (horizontal != ResizeMode::Fixed)
    || (vertical != ResizeMode::Fixed);
  SDL_SetWindowResizable(mSDLWindow, anyAllow);
}

Size LinuxWindow::GetCanvasSize() const {
  if (!mSDLWindow) {
    return {};
  }
  int w {}, h {};
  SDL_GetWindowSize(mSDLWindow, &w, &h);
  return {static_cast<float>(w), static_cast<float>(h)};
}

float LinuxWindow::GetDPIScale() const {
  if (!mSDLWindow) {
    return 1.0f;
  }
  const auto scale = SDL_GetWindowPixelDensity(mSDLWindow);
  return scale > 0.0f ? scale : 1.0f;
}

NativePoint LinuxWindow::CanvasPointToNativePoint(const Point& canvas) const {
  return NativePoint {
    static_cast<int32_t>(std::lround(canvas.mX)),
    static_cast<int32_t>(std::lround(canvas.mY)),
  };
}

Point LinuxWindow::NativePointToCanvasPoint(const NativePoint& native) const {
  return Point {
    static_cast<float>(native.mX),
    static_cast<float>(native.mY),
  };
}

void LinuxWindow::ResizeIfNeeded() {
  // Auto-fit popups to their content. FUI's cross-platform popup flow
  // doesn't call ResizeToFit for tooltips/menus — Win32Window picks up
  // the right size at HWND creation via GetInitialWindowSize. We don't
  // know the widget tree at SDL_CreatePopupWindow time (the popup body
  // runs *between* BeginFrame and EndFrame, after the SDL window is
  // already up), so do the resize each frame from ResizeIfNeeded
  // instead. ResizeIfNeeded runs at the top of Window::Paint, which is
  // far enough into the frame that the widget tree is populated and
  // YGNodeCalculateLayout in ResizeToIdeal returns realistic content
  // sizes. Cost is one Yoga layout pass per popup frame; popups stay
  // open briefly so this is fine.
  if (mPopupParent) {
    this->ResizeToIdeal();
  }
  // Tooltip popups must be mouse-passthrough (Win32's WS_EX_TRANSPARENT
  // analogue). On Wayland the input region is double-buffered state that
  // SDL3 may reset during its own surface bookkeeping, so re-stage the
  // empty region every frame; SDL3's next commit delivers it. No-op on X11.
  if (mIsTooltip && mSDLWindow) {
    Linux::RestakeTooltipInputRegion(mSDLWindow);
  }
  // The popup-fit above may have called SDL_SetWindowSize, so the backend
  // hook runs after it — that's where the swapchain reads the new SDL
  // pixel size and rebuilds.
  this->ResizeBackend();
  mNeedsResize.store(false, std::memory_order_release);
}

Color LinuxWindow::GetClearColor() const {
  return Color {StaticTheme::Common::SolidBackgroundFillColorBase.Resolve(
    StaticTheme::GetCurrent())};
}

void LinuxWindow::SetParent(NativeHandle nh) {
  // Stash the parent SDL_Window* for InitializeWindow to consume. SDL
  // popups must be created with their parent at creation time —
  // SDL_CreatePopupWindow has no equivalent of "reparent later".
  mPopupParent = static_cast<SDL_Window*>(nh.mValue);
}

void LinuxWindow::OffsetPositionToDescendant(Widgets::Widget*) {
  // Used to re-anchor a popup; not yet implemented.
}

void LinuxWindow::ResizeToIdeal() {
  if (!mSDLWindow) {
    return;
  }
  auto* const immediateRoot = this->GetRoot()->GetImmediateRoot();
  if (!immediateRoot) {
    return;
  }
  const auto yoga = immediateRoot->GetLayoutNode();
  // Force a fresh layout pass so we read this-frame sizes rather than
  // last-frame's. Without this, popups that were just created have
  // YGNodeLayoutGet* == 0 and the SDL window snaps to zero size before
  // it ever paints. Pass YGUndefined to let Yoga measure to content.
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);

  const float w = YGNodeLayoutGetWidth(yoga);
  const float h = YGNodeLayoutGetHeight(yoga);
  if (w <= 0 || h <= 0) {
    return;
  }
  SDL_SetWindowSize(
    mSDLWindow,
    static_cast<int>(std::ceil(w)),
    static_cast<int>(std::ceil(h)));
}

bool LinuxWindow::IsDisabled() const {
  return false;
}

bool LinuxWindow::IsPopup() const noexcept {
  return mPopupParent != nullptr;
}

void LinuxWindow::SetIsToolTip() {
  // Must be called BEFORE InitializeWindow; SDL_CreatePopupWindow's
  // tooltip flag is fixed at creation. SetIsToolTip happens early in
  // FUI's tooltip widget setup, well before BeginFrame, so the
  // ordering is fine in practice.
  mIsTooltip = true;
}

void LinuxWindow::SetBackdrop(const WindowBackdrop&) {
  // Linux has no Mica/Acrylic; solid color is what we get. See plan.md §0.5.
}

}// namespace FredEmmott::GUI
