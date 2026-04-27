// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux TextBox: full-featured text input. SDL3 drives windowing/IME,
// ICU drives word/grapheme breaks, Skia (via Font::MeasureTextWidth)
// drives metrics. Mirrors the cross-platform Widgets/TextBox.cpp shape
// with the TSF/COM/UIA bits stripped out — those are handled by SDL3
// text-input + AT-SPI2 (still TBD) on Linux.

#include <FredEmmott/GUI/Widgets/TextBox.hpp>

#include <SDL3/SDL.h>
#include <Yoga.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <ranges>

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/FocusManager.hpp>
#include <FredEmmott/GUI/Renderer.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/StaticTheme/TextBox.hpp>
#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <FredEmmott/GUI/detail/icu.hpp>
#include <FredEmmott/GUI/events/KeyCode.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <FredEmmott/GUI/events/MouseEvent.hpp>
#include <FredEmmott/GUI/events/TextInputEvent.hpp>

namespace FredEmmott::GUI::Widgets {

// Linux has no TSF/UIA equivalent wired up yet (AT-SPI2 is the eventual
// path, see plan.md §3.4). Keep the type so the header doesn't need
// platform gates, but it stores nothing.
struct TextBox::Automation {};

namespace {
constexpr LiteralStyleClass TextBoxStyleClass("TextBox");
constexpr LiteralStyleClass TextContainerStyleClass("TextBox/Text");

auto& TextContainerStyles() {
  static const ImmutableStyle ret {Style().FlexGrow(1)};
  return ret;
}

// Compose DefaultTextBoxStyle with a MinHeight that matches the WinUI3
// standard so the box has a sensible default size. The cross-platform
// widget gets its height from a Yoga measure callback on its text
// container; we don't run that callback here (no font shaping at layout
// time on Linux yet), so a fixed MinHeight stands in.
const ImmutableStyle& StubbedTextBoxStyle() {
  static const ImmutableStyle ret {
    Style().MinHeight(32) + StaticTheme::TextBox::DefaultTextBoxStyle(),
  };
  return ret;
}

bool IsWordCharacter(UText* text, std::size_t index) {
  const auto c = utext_char32At(text, index);
  return u_isalnum(c) || u_hasBinaryProperty(c, UCHAR_IDEOGRAPHIC);
}

UBreakIterator* LazyUBreakIterator(
  felly::unique_ptr<UBreakIterator, &ubrk_close>& owned,
  const UBreakIteratorType iteratorType,
  UText* text) noexcept {
  if (owned) {
    return owned.get();
  }
  UErrorCode status = U_ZERO_ERROR;
  owned.reset(ubrk_open(iteratorType, nullptr, nullptr, 0, &status));
  ubrk_setUText(owned.get(), text, &status);
  return owned.get();
}
}// namespace

TextBox::TextBox(Window* const window)
  : Widget(
      window,
      TextBoxStyleClass,
      StubbedTextBoxStyle(),
      {PseudoClasses::ExplicitMouseButtonSink}),
    IFocusable(this),
    mAutomation(std::make_unique<Automation>()) {
  // Mirror cross-platform layout: a flex-grow placeholder takes the text
  // area so the immediate-mode wrapper's clear-button (a logical child)
  // gets pushed to the right edge instead of laying out where the user
  // clicks to type. Without this the clear button lands at the textbox's
  // left edge and a double-click on the text area triggers it.
  mTextContainer
    = new Widget(window, TextContainerStyleClass, TextContainerStyles());
  this->SetStructuralChildren({mTextContainer});
}

TextBox::~TextBox() = default;

void TextBox::SetText(const std::string_view text) {
  auto& s = mActiveState;
  if (text == s.mText) {
    return;
  }
  mWasChanged = true;
  s.mText = std::string {text};
  mCaches = {};
  // Clamp existing selection into new bounds rather than collapsing it to
  // the end — IME composition replaces a known range and SetSelection is
  // called explicitly afterwards.
  this->SetSelection(s.mSelectionStart, s.mSelectionEnd);
}

FrameRateRequirement TextBox::GetFrameRateRequirement() const noexcept {
  if (!mIsFocused) {
    return Widget::GetFrameRateRequirement();
  }
  if (mActiveState.mSelectionStart != mActiveState.mSelectionEnd) {
    return Widget::GetFrameRateRequirement();
  }
  const auto interval = SystemSettings::Get().GetCaretBlinkInterval();
  if (!interval) {
    return Widget::GetFrameRateRequirement();
  }
  return FrameRateRequirement::After {mLastCaretToggleAt + *interval};
}

// Wide-string accessors are TSF-shaped and unused on Linux. AT-SPI2 will
// provide its own UTF-8 access path when accessibility lands.
std::wstring_view TextBox::GetTextW() const noexcept {
  return {};
}
void TextBox::SetTextW(std::wstring_view) {}
std::pair<std::size_t, std::size_t> TextBox::GetSelectionW() const {
  return {0, 0};
}
void TextBox::SetSelectionW(std::size_t, std::size_t) {}
TextBox::BoundingBox TextBox::GetTextBoundingBoxW(std::size_t, std::size_t)
  const noexcept {
  return {};
}

void TextBox::SelectAll() {
  this->SetSelection(0, mActiveState.mText.size());
}

void TextBox::DeleteSelection(const DeleteDirection ifSelectionEmpty) {
  auto& s = mActiveState;
  if (s.mText.empty() && s.mSelectionStart == s.mSelectionEnd) {
    return;
  }
  if (s.mSelectionStart == s.mSelectionEnd) {
    switch (ifSelectionEmpty) {
      case DeleteDirection::DeleteLeft:
        if (s.mSelectionEnd > 0) {
          s.mSelectionEnd
            = ubrk_preceding(GetGraphemeIterator(), s.mSelectionEnd);
        }
        break;
      case DeleteDirection::DeleteRight:
        if (s.mSelectionEnd < s.mText.size()) {
          s.mSelectionEnd
            = ubrk_following(GetGraphemeIterator(), s.mSelectionEnd);
        }
        break;
    }
  }
  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);
  if (left == right) {
    return;
  }
  s.mText.erase(left, right - left);
  mWasChanged = true;
  mCaches = {};
  this->SetSelection(left, left);
}

Rect TextBox::GetContentRect() const noexcept {
  const auto yoga = this->GetLayoutNode();
  const Rect outerRect {Point {}, this->GetSize()};
  return outerRect.WithInset(
    YGNodeLayoutGetPadding(yoga, YGEdgeLeft)
      + YGNodeLayoutGetBorder(yoga, YGEdgeLeft),
    YGNodeLayoutGetPadding(yoga, YGEdgeTop)
      + YGNodeLayoutGetBorder(yoga, YGEdgeTop),
    YGNodeLayoutGetPadding(yoga, YGEdgeRight)
      + YGNodeLayoutGetBorder(yoga, YGEdgeRight),
    YGNodeLayoutGetPadding(yoga, YGEdgeBottom)
      + YGNodeLayoutGetBorder(yoga, YGEdgeBottom));
}

void TextBox::Tick(const std::chrono::steady_clock::time_point& now) {
  const auto window = this->GetOwnerWindow();
  if (!window) {
    return;
  }
  const auto fm = window->GetFocusManager();
  if (!fm) {
    return;
  }
  const auto isFocused = fm->IsWidgetFocused(this);
  const bool focusChanged = (isFocused != mIsFocused);
  mIsFocused = isFocused;

  // Focus-driven SDL text-input toggle. SDL3 only emits SDL_EVENT_TEXT_INPUT
  // between SDL_StartTextInput and SDL_StopTextInput.
  if (focusChanged) {
    auto* const sdl = static_cast<SDL_Window*>(window->GetNativeHandle().mValue);
    if (sdl) {
      if (mIsFocused) {
        SDL_StartTextInput(sdl);
      } else {
        SDL_StopTextInput(sdl);
      }
    }
    mCaretVisible = true;
    mLastCaretToggleAt = now;
    return;
  }

  // Caret blink — only when focused with a collapsed selection.
  const auto& s = mActiveState;
  const bool hasCaret = (s.mSelectionStart == s.mSelectionEnd);
  const auto blinkInterval = SystemSettings::Get().GetCaretBlinkInterval();
  if (!(isFocused && hasCaret && blinkInterval.has_value())) {
    return;
  }
  if (now - mLastCaretToggleAt < *blinkInterval) {
    return;
  }
  mCaretVisible = !mCaretVisible;
  mLastCaretToggleAt += *blinkInterval;
}

Widget::EventHandlerResult TextBox::OnTextInput(const TextInputEvent& e) {
  if (e.mText.empty()) {
    return EventHandlerResult::StopPropagation;
  }
  this->ReplaceSelection(e.mText, UndoableState::Operation::Typing);
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult TextBox::OnKeyPress(const KeyPressEvent& e) {
  switch (e.mKeyCode) {
    case KeyCode::Key_Backspace:
      this->DeleteSelection(DeleteDirection::DeleteLeft);
      return EventHandlerResult::StopPropagation;
    case KeyCode::Key_Delete:
      this->DeleteSelection(DeleteDirection::DeleteRight);
      return EventHandlerResult::StopPropagation;
    default:
      break;
  }
  return Widget::OnKeyPress(e);
}

void TextBox::PaintCursor(
  Renderer* const renderer,
  const Rect& rect,
  const Style& style) const {
  const auto window = this->GetOwnerWindow();
  if (!window || !window->GetFocusManager()->IsWidgetFocused(this)) {
    return;
  }
  if (!mCaretVisible) {
    return;
  }
  const auto& metrics = this->GetMetrics();
  const auto& s = mActiveState;
  const auto midX = metrics.mOffsetX[s.mSelectionStart];
  const auto width = static_cast<float>(SystemSettings::Get().GetCaretWidth());
  const auto left = midX - (width / 2.0f);
  renderer->FillRect(
    style.Color().value(),
    Rect {
      Point {rect.GetLeft() + left, rect.GetTop()},
      Size {width, rect.GetHeight()},
    });
}

const TextBox::TextMetrics& TextBox::GetMetrics() const {
  if (mCaches.mTextMetrics) {
    return mCaches.mTextMetrics.value();
  }
  // Resolve the same font PaintOwnContent uses, so cumulative widths match
  // what's actually drawn (otherwise IndexFromLocalX would land in the
  // wrong character on a click).
  const auto font = this->GetComputedStyle().Font().value_or(
    SystemFont::Resolve(SystemFont::Usage::Body));
  const auto& fontMetrics = font.GetMetrics();
  TextMetrics ret {
    .mOffsetX = {0.0f},
    .mAscent = fontMetrics.mAscent,
    .mDescent = fontMetrics.mDescent,
  };
  const auto& text = mActiveState.mText;
  ret.mOffsetX.reserve(text.size() + 1);

  // Walk grapheme boundaries and snapshot the cumulative width at each
  // boundary's byte index. Bytes inside a grapheme get sNaN — we never
  // index them (selection / caret positions snap to grapheme boundaries).
  const auto it = GetGraphemeIterator();
  ubrk_first(it);
  for (int32_t next = ubrk_next(it); next != UBRK_DONE; next = ubrk_next(it)) {
    const auto width
      = font.MeasureTextWidth(text.substr(0, static_cast<std::size_t>(next)));
    if (next > 0) {
      ret.mOffsetX.resize(next, std::numeric_limits<float>::signaling_NaN());
    }
    ret.mOffsetX.emplace_back(width);
  }
  mCaches.mTextMetrics.emplace(std::move(ret));
  return mCaches.mTextMetrics.value();
}

void TextBox::BeforeOperation(const UndoableState::Operation op) {
  if (op == mActiveState.mOperation) {
    return;
  }
  mUndoState = mActiveState;
  mUndoState.mOperation = UndoableState::Operation::None;
  mActiveState.mOperation = op;
  mCaches = {};
}

void TextBox::PaintOwnContent(
  Renderer* const renderer,
  const Rect&,
  const Style& style) const {
  const auto& s = mActiveState;
  const auto rect = this->GetContentRect();

  const auto& font = style.Font().value_or(
    SystemFont::Resolve(SystemFont::Usage::Body));
  const auto& color = style.Color().value_or(
    StaticTheme::Common::TextFillColorPrimaryBrush.Resolve(
      StaticTheme::GetCurrent()));

  if (s.mText.empty()) {
    this->PaintCursor(renderer, rect, style);
    return;
  }

  const auto& metrics = this->GetMetrics();
  Point origin {
    rect.GetLeft(),
    rect.GetBottom() - metrics.mDescent,
  };

  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);

  if (left > 0) {
    const auto prefix = s.mText.substr(0, left);
    const auto w = metrics.mOffsetX[left];
    renderer->DrawText(color, rect, font, prefix, origin);
    origin.mX += w;
  }

  if (left == right) {
    this->PaintCursor(renderer, rect, style);
  } else {
    const auto selectionBox = this->GetTextBoundingBox(left, right).mRect;
    renderer->FillRect(Colors::Blue, selectionBox);
    const auto selection = s.mText.substr(left, right - left);
    const auto w = metrics.mOffsetX[right] - metrics.mOffsetX[left];
    renderer->DrawText(Colors::White, rect, font, selection, origin);
    origin.mX += w;
  }

  if (right < s.mText.size()) {
    const auto suffix = s.mText.substr(right);
    renderer->DrawText(color, rect, font, suffix, origin);
  }
}

void TextBox::SetSelection(const std::size_t start, const std::size_t end) {
  auto& s = mActiveState;
  const auto size = s.mText.size();
  s.mSelectionStart = std::min(start, size);
  s.mSelectionEnd = std::min(end, size);
  mCaretVisible = true;
  mLastCaretToggleAt = std::chrono::steady_clock::now();
  // Horizontal scrolling on overflow is a follow-up; the current Linux
  // stub assumes the text fits the box.
  mContentScrollX = 0;
}

void TextBox::ReplaceSelection(
  const std::string_view newContent,
  const UndoableState::Operation op) {
  this->BeforeOperation(op);
  auto& s = mActiveState;
  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);
  s.mText.replace(left, right - left, newContent);
  mWasChanged = true;
  mCaches = {};
  this->SetSelection(left + newContent.size(), left + newContent.size());
}

UText* TextBox::GetUText() const noexcept {
  if (!mCaches.mUText) {
    const auto& s = mActiveState;
    UErrorCode status = U_ZERO_ERROR;
    mCaches.mUText.reset(
      utext_openUTF8(nullptr, s.mText.data(), s.mText.size(), &status));
  }
  return mCaches.mUText.get();
}

UBreakIterator* TextBox::GetGraphemeIterator() const noexcept {
  return LazyUBreakIterator(
    mCaches.mGraphemeIterator, UBRK_CHARACTER, GetUText());
}

UBreakIterator* TextBox::GetWordIterator() const noexcept {
  return LazyUBreakIterator(mCaches.mWordIterator, UBRK_WORD, GetUText());
}

std::size_t TextBox::GetPreviousWordBoundary() const noexcept {
  const auto it = GetWordIterator();
  for (auto idx = ubrk_preceding(it, mActiveState.mSelectionEnd);
       idx != UBRK_DONE;
       idx = ubrk_previous(it)) {
    if (IsWordCharacter(GetUText(), idx)) {
      return idx;
    }
  }
  return 0;
}

std::size_t TextBox::GetNextWordBoundary() const noexcept {
  const auto it = GetWordIterator();
  for (auto idx = ubrk_following(it, mActiveState.mSelectionStart);
       idx != UBRK_DONE;
       idx = ubrk_next(it)) {
    if (IsWordCharacter(GetUText(), idx)) {
      return idx;
    }
  }
  return mActiveState.mText.size();
}

std::size_t TextBox::IndexFromLocalX(const float x) const noexcept {
  const auto yoga = this->GetLayoutNode();
  const float leftInset = YGNodeLayoutGetPadding(yoga, YGEdgeLeft)
    + YGNodeLayoutGetBorder(yoga, YGEdgeLeft);
  const float contentX = x - leftInset;

  const auto& offsets = this->GetMetrics().mOffsetX;
  if (contentX <= 0) {
    return 0;
  }
  if (contentX >= offsets.back()) {
    return mActiveState.mText.size();
  }

  // Walk grapheme boundaries (skip sNaN slots) and pick the closest one.
  std::size_t closestIndex = 0;
  float closestDistance = contentX;
  for (auto&& [index, offset]: std::views::enumerate(offsets)) {
    if (std::isnan(offset)) {
      continue;
    }
    const auto distance = std::abs(offset - contentX);
    if (distance < closestDistance) {
      closestDistance = distance;
      closestIndex = static_cast<std::size_t>(index);
    }
    if (offset >= contentX) {
      break;
    }
  }
  return closestIndex;
}

TextBox::BoundingBox TextBox::GetTextBoundingBox(
  const std::size_t begin,
  const std::size_t end) const noexcept {
  const auto contentRect = this->GetContentRect();
  const auto& metrics = this->GetMetrics();
  const auto left = contentRect.GetLeft() + metrics.mOffsetX[begin];
  auto width = metrics.mOffsetX[end] - metrics.mOffsetX[begin];
  const bool clipped = (width > contentRect.GetWidth()) || (left < 0);
  if (clipped) {
    width = contentRect.GetWidth();
  }
  return BoundingBox {
    Rect {
      Point {left, contentRect.GetTop()},
      Size {
        width,
        metrics.mDescent - /* always negative, so addition */ metrics.mAscent,
      },
    },
    clipped,
  };
}

YGSize TextBox::Measure(const YGNode*, float, YGMeasureMode, float, YGMeasureMode) {
  // Linux stub doesn't measure via Yoga — MinHeight on the outer style and
  // FlexGrow on the text container do the layout work. Defined to satisfy
  // the header's static-method declaration.
  return {0, 0};
}

Widget::EventHandlerResult TextBox::OnMouseButtonPress(const MouseEvent& e) {
  using enum EventHandlerResult;
  if (!e.IsValid()) {
    return Default;
  }
  (void)Widget::OnMouseButtonPress(e);
  if (this->IsDisabled()) {
    return Default;
  }

  // Give focus first so subsequent text-input + caret work.
  if (const auto window = this->GetOwnerWindow()) {
    if (const auto fm = window->GetFocusManager()) {
      fm->GiveImplicitFocus(this);
    }
  }

  std::uint8_t clicks = 1;
  if (const auto* press = std::get_if<MouseEvent::ButtonPressEvent>(&e.mDetail)) {
    clicks = press->mClickCount;
  }

  const auto pos = e.GetPosition();
  const auto idx = this->IndexFromLocalX(pos.mX);

  // Triple-click → select all. Double-click → select word at click. Single
  // click → set caret + start drag-selection. WinUI3 / Win32 EditCtrl
  // behavior; ICU's UBRK_WORD gives us the boundary set.
  if (clicks >= 3) {
    this->SelectAll();
    mMouseSelectionAnchor.reset();
    return StopPropagation;
  }
  if (clicks == 2 && !mActiveState.mText.empty()) {
    // Probe one byte before end-of-text when the click lands past the
    // last char, so the last word selects instead of collapsing to an
    // empty range at .size().
    const auto it = GetWordIterator();
    const auto textLen = static_cast<int32_t>(mActiveState.mText.size());
    const auto probe
      = std::min(static_cast<int32_t>(idx), std::max(0, textLen - 1));
    auto wordStart = ubrk_preceding(it, probe + 1);
    auto wordEnd = ubrk_following(it, probe);
    if (wordStart == UBRK_DONE) {
      wordStart = 0;
    }
    if (wordEnd == UBRK_DONE) {
      wordEnd = textLen;
    }
    this->SetSelection(
      static_cast<std::size_t>(wordStart),
      static_cast<std::size_t>(wordEnd));
    mMouseSelectionAnchor.reset();
    return StopPropagation;
  }

  // Single click → caret + start drag.
  mMouseSelectionAnchor = idx;
  this->SetSelection(idx, idx);
  this->StartMouseCapture();
  return StopPropagation;
}

Widget::EventHandlerResult TextBox::OnMouseMove(const MouseEvent& e) {
  using enum EventHandlerResult;
  if (!mMouseSelectionAnchor.has_value()) {
    return Default;
  }
  const auto pos = e.GetPosition();
  const auto idx = this->IndexFromLocalX(pos.mX);
  this->SetSelection(*mMouseSelectionAnchor, idx);
  return StopPropagation;
}

Widget::EventHandlerResult TextBox::OnMouseButtonRelease(const MouseEvent& e) {
  using enum EventHandlerResult;
  if (!mMouseSelectionAnchor.has_value()) {
    return Default;
  }
  (void)Widget::OnMouseButtonRelease(e);
  mMouseSelectionAnchor.reset();
  this->EndMouseCapture();
  return StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
