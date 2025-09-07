// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

#include <print>

#include "FredEmmott/GUI/FocusManager.hpp"
#include "FredEmmott/GUI/SystemSettings.hpp"
#include "FredEmmott/GUI/detail/icu.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "FredEmmott/GUI/events/KeyEvent.hpp"

namespace FredEmmott::GUI::Widgets {
namespace {
using namespace win32_detail;

bool IsWordCharacter(UText* text, std::size_t index) {
  const auto c = utext_char32At(text, index);
  return u_isalnum(c) || u_hasBinaryProperty(c, UCHAR_IDEOGRAPHIC);
}

auto& TextBoxStyles() {
  static const ImmutableStyle ret {
    Style().FlexDirection(YGFlexDirectionRow),
  };
  return ret;
}
}// namespace

TextBox::TextBox(const std::size_t id) : Widget(id, TextBoxStyles(), {}) {
  mWindow = Immediate::immediate_detail::tWindow;

  YGNodeSetMeasureFunc(this->GetLayoutNode(), &TextBox::Measure);

  this->SetText("H💩ello, world");
}

TextBox::~TextBox() = default;

void TextBox::SetText(const std::string_view text) {
  auto& s = mActiveState;
  if (text == s.mText) {
    return;
  }

  mCaches = {};

  s.mText = std::string {text};
  this->SetSelection(s.mSelectionStart, s.mSelectionEnd);
  YGNodeMarkDirty(this->GetLayoutNode());

  // TODO: notify IME
}

void TextBox::Tick() {
  const auto isFocused = FocusManager::IsWidgetFocused(this);
  if (isFocused == mIsFocused) {
    return;
  }
  mIsFocused = isFocused;

  // TODO: notify IME
}

Widget::EventHandlerResult TextBox::OnTextInput(const TextInputEvent& e) {
  using enum EventHandlerResult;
  auto& s = mActiveState;
  const auto& t = e.mText;

  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);

  BeforeOperation(UndoableState::Operation::Typing);
  this->SetText(
    std::format("{}{}{}", s.mText.substr(0, left), t, s.mText.substr(right)));
  this->SetCaret(left + t.size());

  return StopPropagation;
}

void TextBox::DeleteSelection(const DeleteDirection ifSelectionEmpty) {
  auto& s = mActiveState;
  if (s.mSelectionStart == s.mSelectionEnd) {
    switch (ifSelectionEmpty) {
      case DeleteLeft:
        if (s.mSelectionEnd > 0) {
          s.mSelectionEnd
            = ubrk_preceding(GetGraphemeIterator(), s.mSelectionEnd);
        }
        break;
      case DeleteRight:
        if (s.mSelectionEnd < s.mText.size()) {
          s.mSelectionEnd
            = ubrk_following(GetGraphemeIterator(), s.mSelectionEnd);
        }
        break;
    }
  }

  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);
  this->SetText(
    std::format("{}{}", s.mText.substr(0, left), s.mText.substr(right)));
  this->SetCaret(left);
}

Widget::EventHandlerResult TextBox::OnKeyPress(const KeyPressEvent& e) {
  using enum KeyModifier;
  using enum EventHandlerResult;

  auto& s = mActiveState;
  std::optional<std::size_t> newIdx;

  if ((e.mModifiers & Modifier_Alt) == Modifier_Alt) {
    return StopPropagation;
  }

  using enum KeyCode;
  switch (e.mKeyCode) {
    case Key_A:
      if (e.mModifiers == Modifier_Control) {
        this->SetSelection(0, s.mText.size());
      }
      return StopPropagation;
    case Key_Z:
      if (e.mModifiers == Modifier_Control) {
        mCaches = {};
        std::swap(mActiveState, mUndoState);
        YGNodeMarkDirty(this->GetLayoutNode());
        // TODO: notify IME
      }
      return StopPropagation;
    case Key_Backspace:
      BeforeOperation(UndoableState::Operation::DeleteLeft);
      this->DeleteSelection(DeleteLeft);
      return StopPropagation;
    case Key_Delete:
      BeforeOperation(UndoableState::Operation::DeleteRight);
      this->DeleteSelection(DeleteRight);
      return StopPropagation;
    case Key_Home:
      newIdx = 0;
      break;
    case Key_End:
      newIdx = s.mText.size();
      break;
    case Key_LeftArrow:
      if (
        (s.mSelectionStart != s.mSelectionEnd)
        && (e.mModifiers == Modifier_None)) {
        newIdx = std::min(s.mSelectionStart, s.mSelectionEnd);
      } else if ((e.mModifiers & Modifier_Control) == Modifier_Control) {
        newIdx = GetPreviousWordBoundary();
      } else if (const auto idx
                 = ubrk_preceding(GetGraphemeIterator(), s.mSelectionEnd);
                 idx != UBRK_DONE) {
        newIdx = idx;
      }
      break;
    case Key_RightArrow:
      if (
        (s.mSelectionStart != s.mSelectionEnd)
        && (e.mModifiers == Modifier_None)) {
        newIdx = std::max(s.mSelectionStart, s.mSelectionEnd);
      } else if ((e.mModifiers & Modifier_Control) == Modifier_Control) {
        newIdx = GetNextWordBoundary();
      } else if (const auto idx
                 = ubrk_following(GetGraphemeIterator(), s.mSelectionEnd);
                 idx != UBRK_DONE) {
        newIdx = idx;
      }
      break;
    default:
      break;
  }
  if (newIdx) {
    if ((e.mModifiers & Modifier_Shift) == Modifier_Shift) {
      this->SetSelection(s.mSelectionStart, *newIdx);
      return StopPropagation;
    }
    this->SetCaret(*newIdx);
    return StopPropagation;
  }

  return StopPropagation;
}

void TextBox::PaintCursor(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  if (!FocusManager::IsWidgetFocused(this)) {
    return;
  }

  const auto& metrics = this->GetMetrics();
  const auto midX = metrics.mWidthBeforeSelection;

  const auto width = static_cast<float>(SystemSettings::Get().GetCaretWidth());
  const auto left = midX - (width / 2);
  renderer->FillRect(
    style.Color().value(),
    Rect {
      Point {
        rect.GetLeft() + left,
        rect.GetTop(),
      },
      Size {
        width,
        rect.GetHeight(),
      },
    });
}

const TextBox::TextMetrics& TextBox::GetMetrics() const {
  if (mCaches.mTextMetrics) {
    return mCaches.mTextMetrics.value();
  }

  const auto& font = this->GetComputedStyle().Font().value();
  const auto& fontMetrics = font.GetMetrics();
  TextMetrics ret {
    .mAscent = fontMetrics.mAscent,
    .mDescent = fontMetrics.mDescent,
  };

  const auto& s = mActiveState;
  const auto [beforeSelection, afterSelection]
    = std::minmax(s.mSelectionStart, s.mSelectionEnd);

  if (beforeSelection > 0) {
    ret.mWidthBeforeSelection
      = font.MeasureTextWidth(s.mText.substr(0, beforeSelection));
  }
  if (beforeSelection != afterSelection) {
    ret.mWidthOfSelection = font.MeasureTextWidth(
      s.mText.substr(beforeSelection, afterSelection - beforeSelection));
  }
  if (afterSelection < s.mText.size()) {
    ret.mWidthAfterSelection
      = font.MeasureTextWidth(s.mText.substr(afterSelection));
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
  Renderer* renderer,
  const Rect& outerRect,
  const Style& style) const {
  const auto& s = mActiveState;

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

  const auto& metrics = this->GetMetrics();
  const auto& color = style.Color().value();
  const auto& font = style.Font().value();

  Point origin {
    rect.GetLeft(),
    rect.GetBottom() - metrics.mDescent,
  };

  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);

  if (left > 0) {
    const auto prefix = s.mText.substr(0, left);
    const auto w = metrics.mWidthBeforeSelection;
    renderer->DrawText(color, rect, font, prefix, origin);
    origin.mX += w;
  }

  if (left == right) {
    this->PaintCursor(renderer, rect, style);
  } else {
    const auto selection = s.mText.substr(left, right - left);
    const auto w = metrics.mWidthOfSelection;
    const auto h = rect.GetHeight();
    renderer->FillRect(
      Colors::Blue,
      Rect {
        Point {origin.mX, origin.mY - h},
        Size {w, h},
      });
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

  mCaches.mTextMetrics.reset();
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

static UBreakIterator* LazyUBreakIterator(
  unique_ptr<UBreakIterator, &ubrk_close>& owned,
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

UBreakIterator* TextBox::GetGraphemeIterator() const noexcept {
  return LazyUBreakIterator(
    mCaches.mGraphemeIterator, UBRK_CHARACTER, GetUText());
}

UBreakIterator* TextBox::GetWordIterator() const noexcept {
  return LazyUBreakIterator(mCaches.mWordIterator, UBRK_WORD, GetUText());
}

std::size_t TextBox::GetPreviousWordBoundary() const noexcept {
  // We want word boundaries *just before* the words, and at the end of the
  // end of the string, e.g.:

  //   foo, bar, baz
  //  ><   ><   >< ><

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

YGSize TextBox::Measure(
  const YGNodeConstRef node,
  [[maybe_unused]] float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  [[maybe_unused]] float height,
  [[maybe_unused]] YGMeasureMode heightMode) {
  const auto& self = *static_cast<TextBox*>(FromYogaNode(node));

  const auto& metrics = self.GetMetrics();

  return {
    metrics.mWidthBeforeSelection + metrics.mWidthOfSelection
      + metrics.mWidthAfterSelection,
    -metrics.mAscent,
  };
}
}// namespace FredEmmott::GUI::Widgets
