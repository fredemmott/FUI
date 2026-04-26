// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Phase 2 Linux TextBox: minimum viable text input. Handles UTF-8
// append on text-input events, backspace, mouse-click → focus, paint of
// the current text. Selection / cursor positioning by mouse / IME
// preedit / undo-redo are deferred to phase 4 — that work shares
// machinery with the cross-platform Widgets/TextBox.cpp which is
// currently Win32-TSF-coupled and not yet portable.

#include "../Widgets/TextBox.hpp"

#include <SDL3/SDL.h>
#include <Yoga.h>

#include <FredEmmott/GUI/FocusManager.hpp>
#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/StaticTheme/TextBox.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/Window.hpp>

#include "../Style.hpp"
#include "../Widgets/Widget.hpp"
#include "../events/KeyCode.hpp"
#include "../events/KeyEvent.hpp"
#include "../events/TextInputEvent.hpp"

namespace FredEmmott::GUI::Widgets {

struct TextBox::Automation {};

namespace {
constexpr LiteralStyleClass TextBoxStyleClass("TextBox");

// Pop the last UTF-8 code point off the end of `s`. Walks back over
// continuation bytes (10xxxxxx) until we find a leading byte. No-op on
// empty string.
void PopUtf8CodePoint(std::string& s) {
  while (!s.empty()) {
    const auto byte = static_cast<unsigned char>(s.back());
    s.pop_back();
    // 0xxxxxxx = ASCII single byte → done.
    // 110xxxxx, 1110xxxx, 11110xxx = leading byte → done.
    // 10xxxxxx = continuation → keep walking back.
    if ((byte & 0xC0) != 0x80) {
      return;
    }
  }
}
}// namespace

// DefaultTextBoxStyle sets padding/border but no MinHeight — the
// cross-platform widget gets its height from a Yoga measure callback
// on its mTextContainer child. Our stub has neither, so a Yoga layout
// pass collapses to padding-only (~13px) and the TextBox renders as a
// thin sliver. Compose DefaultTextBoxStyle with a MinHeight that
// matches the WinUI3 standard so the box has a sensible default size
// until the cross-platform widget arrives.
const ImmutableStyle& StubbedTextBoxStyle() {
  static const ImmutableStyle ret {
    Style().MinHeight(32) + StaticTheme::TextBox::DefaultTextBoxStyle(),
  };
  return ret;
}

TextBox::TextBox(Window* const window)
  : Widget(
      window,
      TextBoxStyleClass,
      StubbedTextBoxStyle(),
      {PseudoClasses::ExplicitMouseButtonSink}),
    IFocusable(this),
    mAutomation(std::make_unique<Automation>()) {
}

TextBox::~TextBox() = default;

void TextBox::SetText(const std::string_view text) {
  auto& s = mActiveState;
  if (text == s.mText) {
    return;
  }
  mWasChanged = true;
  s.mText = std::string {text};
  s.mSelectionStart = s.mText.size();
  s.mSelectionEnd = s.mText.size();
  mCaches = {};
}

FrameRateRequirement TextBox::GetFrameRateRequirement() const noexcept {
  return Widget::GetFrameRateRequirement();
}

// Wide-string accessors are TSF-shaped and unused on Linux for now.
std::wstring_view TextBox::GetTextW() const noexcept {
  return {};
}
void TextBox::SetTextW(std::wstring_view) {}
std::pair<std::size_t, std::size_t> TextBox::GetSelectionW() const {
  return {0, 0};
}
void TextBox::SetSelectionW(std::size_t, std::size_t) {}
void TextBox::SelectAll() {}
TextBox::BoundingBox TextBox::GetTextBoundingBoxW(std::size_t, std::size_t)
  const noexcept {
  return {};
}
void TextBox::DeleteSelection(DeleteDirection) {
  if (mActiveState.mText.empty()) {
    return;
  }
  PopUtf8CodePoint(mActiveState.mText);
  mActiveState.mSelectionStart = mActiveState.mText.size();
  mActiveState.mSelectionEnd = mActiveState.mText.size();
  mWasChanged = true;
  mCaches = {};
}

Rect TextBox::GetContentRect() const noexcept {
  return Rect {Point {}, this->GetSize()};
}

// Drives the SDL3 text-input enable/disable side-effect on focus
// changes. SDL only emits SDL_EVENT_TEXT_INPUT to a window between
// SDL_StartTextInput and SDL_StopTextInput calls; widgets like ours
// must explicitly request input when focused.
void TextBox::Tick(const std::chrono::steady_clock::time_point&) {
  const auto window = this->GetOwnerWindow();
  if (!window) {
    return;
  }
  const auto fm = window->GetFocusManager();
  if (!fm) {
    return;
  }
  const auto isFocused = fm->IsWidgetFocused(this);
  if (isFocused == mIsFocused) {
    return;
  }
  mIsFocused = isFocused;
  auto* const sdl = static_cast<SDL_Window*>(window->GetNativeHandle().mValue);
  if (!sdl) {
    return;
  }
  if (mIsFocused) {
    SDL_StartTextInput(sdl);
  } else {
    SDL_StopTextInput(sdl);
  }
}

Widget::EventHandlerResult TextBox::OnTextInput(const TextInputEvent& e) {
  // Note: no YGNodeMarkDirty here. Yoga rejects MarkDirty on nodes that
  // aren't leaves with a custom measure function ("Only leaf nodes with
  // custom measure functions should manually mark themselves as dirty"),
  // and this stub doesn't register one — TextBox uses the static style's
  // fixed size. Text changes only invalidate paint, not layout.
  if (e.mText.empty()) {
    return EventHandlerResult::StopPropagation;
  }
  mActiveState.mText.append(e.mText);
  mActiveState.mSelectionStart = mActiveState.mText.size();
  mActiveState.mSelectionEnd = mActiveState.mText.size();
  mWasChanged = true;
  mCaches = {};
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult TextBox::OnKeyPress(const KeyPressEvent& e) {
  switch (e.mKeyCode) {
    case KeyCode::Key_Backspace:
      this->DeleteSelection(DeleteDirection::DeleteLeft);
      return EventHandlerResult::StopPropagation;
    default:
      break;
  }
  return Widget::OnKeyPress(e);
}

void TextBox::PaintOwnContent(
  Renderer* const renderer,
  const Rect& outerRect,
  const Style& style) const {
  const auto yoga = this->GetLayoutNode();
  const Rect rect = outerRect.WithInset(
    YGNodeLayoutGetPadding(yoga, YGEdgeLeft)
      + YGNodeLayoutGetBorder(yoga, YGEdgeLeft),
    YGNodeLayoutGetPadding(yoga, YGEdgeTop)
      + YGNodeLayoutGetBorder(yoga, YGEdgeTop),
    YGNodeLayoutGetPadding(yoga, YGEdgeRight)
      + YGNodeLayoutGetBorder(yoga, YGEdgeRight),
    YGNodeLayoutGetPadding(yoga, YGEdgeBottom)
      + YGNodeLayoutGetBorder(yoga, YGEdgeBottom));

  const auto font = style.Font().value_or(
    SystemFont::Resolve(SystemFont::Usage::Body));
  const auto color = style.Color().value_or(
    StaticTheme::Common::TextFillColorPrimaryBrush.Resolve(
      StaticTheme::GetCurrent()));

  const auto metrics = font.GetMetrics();
  const Point baseline {
    rect.GetLeft(),
    rect.GetTop() - metrics.mAscent,
  };

  const auto& text = mActiveState.mText;
  if (!text.empty()) {
    renderer->DrawText(color, rect, font, text, baseline);
  }

  // Caret: draw a 1px vertical line at the end of the text when focused.
  // Real caret blinking + positioning is phase 4.
  if (mIsFocused) {
    const auto textWidth = font.MeasureTextWidth(text);
    const Rect caretRect {
      Point {rect.GetLeft() + textWidth, rect.GetTop()},
      Size {1.0f, metrics.mLineSpacing},
    };
    renderer->FillRect(color, caretRect);
  }
}

Widget::EventHandlerResult TextBox::OnMouseButtonPress(const MouseEvent& e) {
  // Click → take implicit focus so subsequent text-input events route here.
  if (const auto window = this->GetOwnerWindow()) {
    if (const auto fm = window->GetFocusManager()) {
      fm->GiveImplicitFocus(this);
    }
  }
  return Widget::OnMouseButtonPress(e);
}

Widget::EventHandlerResult TextBox::OnMouseMove(const MouseEvent& e) {
  return Widget::OnMouseMove(e);
}

Widget::EventHandlerResult TextBox::OnMouseButtonRelease(const MouseEvent& e) {
  return Widget::OnMouseButtonRelease(e);
}

}// namespace FredEmmott::GUI::Widgets
