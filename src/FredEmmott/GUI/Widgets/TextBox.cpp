// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

#include "FredEmmott/GUI/FocusManager.hpp"
#include "FredEmmott/GUI/SystemSettings.hpp"
#include "FredEmmott/GUI/detail/icu.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "FredEmmott/GUI/events/KeyEvent.hpp"

namespace FredEmmott::GUI::Widgets {
namespace {
using namespace win32_detail;

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
  if (text == mText) {
    return;
  }

  mGraphemeIterator.reset();
  mUText.reset();
  mTextMetrics.reset();

  mText = std::string {text};
  this->SetSelection(mSelectionStart, mSelectionEnd);
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
  const auto& t = e.mText;

  const auto [left, right] = std::minmax(mSelectionStart, mSelectionEnd);

  this->SetText(
    std::format("{}{}{}", mText.substr(0, left), t, mText.substr(right)));
  this->SetCaret(left + t.size());

  return StopPropagation;
}

void TextBox::DeleteSelection(const DeleteDirection ifSelectionEmpty) {
  if (mSelectionStart == mSelectionEnd) {
    switch (ifSelectionEmpty) {
      case DeleteLeft:
        if (mSelectionEnd > 0) {
          mSelectionEnd = ubrk_preceding(GetGraphemeIterator(), mSelectionEnd);
        }
        break;
      case DeleteRight:
        if (mSelectionEnd < mText.size()) {
          mSelectionEnd = ubrk_following(GetGraphemeIterator(), mSelectionEnd);
        }
        break;
    }
  }

  const auto [left, right] = std::minmax(mSelectionStart, mSelectionEnd);
  this->SetText(
    std::format("{}{}", mText.substr(0, left), mText.substr(right)));
  this->SetCaret(left);
}

Widget::EventHandlerResult TextBox::OnKeyPress(const KeyPressEvent& e) {
  using enum KeyModifier;
  std::optional<std::size_t> newIdx;

  using enum KeyCode;
  using enum EventHandlerResult;
  switch (e.mKeyCode) {
    case Key_A:
      if (e.mModifiers == Modifier_Control) {
        this->SetSelection(0, mText.size());
      }
      return StopPropagation;
    case Key_Backspace:
      this->DeleteSelection(DeleteLeft);
      return StopPropagation;
    case Key_Delete:
      this->DeleteSelection(DeleteRight);
      return StopPropagation;
    case Key_Home:
      newIdx = 0;
      break;
    case Key_End:
      newIdx = mText.size();
      break;
    case Key_LeftArrow:
      if (const auto idx = ubrk_preceding(GetGraphemeIterator(), mSelectionEnd);
          idx != UBRK_DONE) {
        newIdx = idx;
      }
      break;
    case Key_RightArrow:
      if (const auto idx = ubrk_following(GetGraphemeIterator(), mSelectionEnd);
          idx != UBRK_DONE) {
        newIdx = idx;
      }
      break;
    default:
      break;
  }
  if (newIdx) {
    if (e.mModifiers == Modifier_None) {
      this->SetCaret(*newIdx);
      return StopPropagation;
    }
    if (e.mModifiers == Modifier_Shift) {
      this->SetSelection(mSelectionStart, *newIdx);
      return StopPropagation;
    }
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
  if (mTextMetrics) {
    return mTextMetrics.value();
  }

  const auto& font = this->GetComputedStyle().Font().value();
  const auto& fontMetrics = font.GetMetrics();
  TextMetrics ret {
    .mAscent = fontMetrics.mAscent,
    .mDescent = fontMetrics.mDescent,
  };

  const auto [beforeSelection, afterSelection]
    = std::minmax(mSelectionStart, mSelectionEnd);

  if (beforeSelection > 0) {
    ret.mWidthBeforeSelection
      = font.MeasureTextWidth(mText.substr(0, beforeSelection));
  }
  if (beforeSelection != afterSelection) {
    ret.mWidthOfSelection = font.MeasureTextWidth(
      mText.substr(beforeSelection, afterSelection - beforeSelection));
  }
  if (afterSelection < mText.size()) {
    ret.mWidthAfterSelection
      = font.MeasureTextWidth(mText.substr(afterSelection));
  }
  mTextMetrics.emplace(std::move(ret));
  return mTextMetrics.value();
}

void TextBox::PaintOwnContent(
  Renderer* renderer,
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

  const auto& metrics = this->GetMetrics();
  const auto& color = style.Color().value();
  const auto& font = style.Font().value();

  Point origin {
    rect.GetLeft(),
    rect.GetBottom() - metrics.mDescent,
  };

  const auto [left, right] = std::minmax(mSelectionStart, mSelectionEnd);

  if (left > 0) {
    const auto prefix = mText.substr(0, left);
    const auto w = metrics.mWidthBeforeSelection;
    renderer->DrawText(color, rect, font, prefix, origin);
    origin.mX += w;
  }

  if (left == right) {
    this->PaintCursor(renderer, rect, style);
  } else {
    const auto selection = mText.substr(left, right - left);
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

  if (right < mText.size()) {
    const auto suffix = mText.substr(right);
    renderer->DrawText(color, rect, font, suffix, origin);
  }
}

void TextBox::SetSelection(const std::size_t start, const std::size_t end) {
  const auto size = mText.size();
  mSelectionStart = std::min(start, size);
  mSelectionEnd = std::min(end, size);
  mTextMetrics.reset();
}

UText* TextBox::GetUText() noexcept {
  if (!mUText) {
    UErrorCode status = U_ZERO_ERROR;
    mUText.reset(utext_openUTF8(nullptr, mText.data(), mText.size(), &status));
  }
  return mUText.get();
}

UBreakIterator* TextBox::GetGraphemeIterator() noexcept {
  if (!mGraphemeIterator) {
    UErrorCode status = U_ZERO_ERROR;
    mGraphemeIterator.reset(
      ubrk_open(UBRK_CHARACTER, nullptr, nullptr, 0, &status));
    ubrk_setUText(mGraphemeIterator.get(), GetUText(), &status);
  }
  return mGraphemeIterator.get();
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
