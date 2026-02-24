// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/TextBox.hpp>

#include "FredEmmott/GUI/FocusManager.hpp"
#include "FredEmmott/GUI/SystemSettings.hpp"
#include "FredEmmott/GUI/Window.hpp"
#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/detail/icu.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail/TSFTextStore.hpp"
#include "FredEmmott/GUI/events/KeyEvent.hpp"
#include "FredEmmott/GUI/events/MouseEvent.hpp"

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

using win32_detail::CheckHResult;
using win32_detail::TextStoreACP;
using win32_detail::TSFThreadManager;

struct TextBox::Automation : TSFThreadManager::Document {
  Automation& operator=(const Document& doc) noexcept {
    static_cast<Document&>(*this) = doc;
    return *this;
  }

  [[nodiscard]]
  ITextStoreACPSink* GetSink(const DWORD mask) const {
    if (!mStore) {
      return nullptr;
    }
    return mStore->GetSink(mask);
  }
};

namespace {
bool IsWordCharacter(UText* text, std::size_t index) {
  const auto c = utext_char32At(text, index);
  return u_isalnum(c) || u_hasBinaryProperty(c, UCHAR_IDEOGRAPHIC);
}

constexpr LiteralStyleClass TextBoxStyleClass("TextBox");
constexpr LiteralStyleClass TextContainerStyleClass("TextBox/Text");
constexpr LiteralStyleClass ButtonsStyleClass("TextBox/Buttons");

auto& TextContainerStyles() {
  static const ImmutableStyle ret {Style().FlexGrow(1)};
  return ret;
}

auto& ButtonContainerStyles() {
  static const ImmutableStyle ret {
    Style().FlexGrow(0).FlexShrink(0).Gap(4),
  };
  return ret;
}
}// namespace

TextBox::TextBox(const std::size_t id)
  : Widget(
      id,
      TextBoxStyleClass,
      StaticTheme::TextBox::DefaultTextBoxStyle(),
      {}),
    mAutomation(std::make_unique<Automation>()) {
  mTextContainer
    = new Widget({}, TextContainerStyleClass, TextContainerStyles());
  mButtons = new Widget({}, ButtonsStyleClass, ButtonContainerStyles());
  this->SetDirectChildren({mTextContainer, mButtons});

  YGNodeSetMeasureFunc(mTextContainer->GetLayoutNode(), &TextBox::Measure);
}

TextBox::~TextBox() = default;

void TextBox::SetText(const std::string_view text) {
  AssertOwnerThread();

  auto& s = mActiveState;
  if (text == s.mText) {
    return;
  }

  mWasChanged = true;
  const auto oldLength = s.mText.size();

  s.mText = std::string {text};
  mCaches = {};
  this->SetSelection(s.mSelectionStart, s.mSelectionEnd);
  YGNodeMarkDirty(mTextContainer->GetLayoutNode());

  if (mAutomationFlag) {
    return;
  }

  if (const auto sink = mAutomation->GetSink(TS_AS_TEXT_CHANGE)) {
    const TS_TEXTCHANGE textChange {
      .acpOldEnd = static_cast<LONG>(oldLength),
      .acpNewEnd = static_cast<LONG>(s.mText.size()),
    };
    CheckHResult(sink->OnTextChange(0, &textChange));
  }
  if (const auto sink = mAutomation->GetSink(TS_AS_LAYOUT_CHANGE)) {
    CheckHResult(sink->OnLayoutChange(TS_LC_CHANGE, 1));
  }
  if (const auto sink = mAutomation->GetSink(TS_AS_SEL_CHANGE)) {
    CheckHResult(sink->OnSelectionChange());
  }
}

FrameRateRequirement TextBox::GetFrameRateRequirement() const noexcept {
  // If caret should blink, request caret-level frame rate
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

std::wstring_view TextBox::GetTextW() const noexcept {
  const auto& text = mActiveState.mText;
  if (text.empty()) {
    return {};
  }
  auto& wideText = mCaches.mWideText;
  if (!wideText.empty()) {
    return wideText;
  }

  wideText = win32_detail::Utf8ToWide(text);
  return wideText;
}

void TextBox::SetTextW(const std::wstring_view wide) {
  SetText(win32_detail::WideToUtf8(wide));
}

Rect TextBox::GetContentRect() const noexcept {
  const auto yoga = this->GetLayoutNode();
  // Full area of the text box
  const Rect outerRect = Rect {
    Point {},
    Size {
      YGNodeLayoutGetWidth(yoga),
      YGNodeLayoutGetHeight(yoga),
    },
  };
  // Where we can render text
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

std::pair<std::size_t, std::size_t> TextBox::GetSelectionW() const {
  using namespace win32_detail;
  const auto [begin, end] = GetSelection();
  const auto wideBegin = Utf8ToWideIndex(mActiveState.mText, begin);
  const auto wideEnd
    = (begin == end) ? wideBegin : Utf8ToWideIndex(mActiveState.mText, end);
  return {wideBegin, wideEnd};
}

void TextBox::SetSelectionW(const std::size_t begin, const std::size_t end) {
  using namespace win32_detail;
  const auto text = GetTextW();
  const auto utf8Begin = WideToUtf8Index(text, begin);
  const auto utf8End = (begin == end) ? utf8Begin : WideToUtf8Index(text, end);
  this->SetSelection(utf8Begin, utf8End);
}

void TextBox::SelectAll() {
  this->SetSelection(0, mActiveState.mText.size());
}

TextBox::BoundingBox TextBox::GetTextBoundingBoxW(
  const std::size_t begin,
  const std::size_t end) const noexcept {
  using namespace win32_detail;
  const auto text = GetTextW();
  const auto utf8Begin = WideToUtf8Index(text, begin);
  const auto utf8End = (begin == end) ? utf8Begin : WideToUtf8Index(text, end);
  return GetTextBoundingBox(utf8Begin, utf8End);
}

Widget* TextBox::GetFosterParent() const noexcept {
  return mButtons;
}

void TextBox::Tick(const std::chrono::steady_clock::time_point& now) {
  const auto isFocused = FocusManager::IsWidgetFocused(this);
  const bool focusChanged = (isFocused != mIsFocused);
  mIsFocused = isFocused;

  // Manage TSF document activation on focus changes
  if (focusChanged) {
    if (mIsFocused) {
      auto& tm = TSFThreadManager::Get();
      const auto hwnd = this->GetOwnerWindow()->GetNativeHandle().mValue;
      tm.Initialize(hwnd);
      *mAutomation = tm.ActivateFor(hwnd, this);
      tm.SetFocus(hwnd, mAutomation.get());
      if (const auto sink = mAutomation->GetSink(TS_AS_LAYOUT_CHANGE)) {
        CheckHResult(sink->OnLayoutChange(TS_LC_CREATE, 1));
      }
    } else {
      auto& tm = TSFThreadManager::Get();
      tm.Deactivate(*mAutomation);
      *mAutomation = {};
    }
  }

  // Blink caret when focused and no selection
  const auto& s = mActiveState;
  const bool hasCaret = (s.mSelectionStart == s.mSelectionEnd);
  const auto blinkInterval = SystemSettings::Get().GetCaretBlinkInterval();

  if (focusChanged) {
    // Reset blink on focus change
    mCaretVisible = true;
    mLastCaretToggleAt = now;
    return;
  }
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
  this->ReplaceSelection(e.mText, UndoableState::Operation::Typing);
  return EventHandlerResult::StopPropagation;
}

void TextBox::DeleteSelection(const DeleteDirection ifSelectionEmpty) {
  AssertOwnerThread();

  using enum DeleteDirection;
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
    case Key_C:
      if (e.mModifiers == Modifier_Control) {
        const auto [left, right]
          = std::minmax(s.mSelectionStart, s.mSelectionEnd);
        GetOwnerWindow()->SetClipboardText(s.mText.substr(left, right - left));
      }
      return StopPropagation;
    case Key_X:
      if (e.mModifiers == Modifier_Control) {
        const auto [left, right]
          = std::minmax(s.mSelectionStart, s.mSelectionEnd);
        GetOwnerWindow()->SetClipboardText(s.mText.substr(left, right - left));
        this->ReplaceSelection({}, UndoableState::Operation::Cut);
      }
      return StopPropagation;
    case Key_V: {
      if (e.mModifiers != Modifier_Control) {
        return StopPropagation;
      }

      if (const auto pasted = GetOwnerWindow()->GetClipboardText()) {
        this->ReplaceSelection(*pasted, UndoableState::Operation::Paste);
      }

      return StopPropagation;
    }
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
      this->DeleteSelection(DeleteDirection::DeleteLeft);
      return StopPropagation;
    case Key_Insert:
      if (e.mModifiers == Modifier_Shift) {
        // Shift-Insert is an alternative paste
        if (const auto pasted = GetOwnerWindow()->GetClipboardText()) {
          this->ReplaceSelection(*pasted, UndoableState::Operation::Paste);
        }
      }
      return StopPropagation;
    case Key_Delete:
      BeforeOperation(UndoableState::Operation::DeleteRight);
      this->DeleteSelection(DeleteDirection::DeleteRight);
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
    case Key_Tab:
      // Tab and shift-tab are for keyboard navigation; ctrl+tab bypasses
      // this to allow typing a tab
      if (e.mModifiers != Modifier_Control) {
        return Widget::OnKeyPress(e);
      }
      this->ReplaceSelection("\t", UndoableState::Operation::Typing);
      return StopPropagation;
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

Widget::EventHandlerResult TextBox::OnMouseButtonPress(const MouseEvent& e) {
  using enum EventHandlerResult;
  if (!e.IsValid()) {
    return Default;
  }
  (void)Widget::OnMouseButtonPress(e);
  if (this->IsDisabled()) {
    return Default;
  }

  // Update caret immediately on press and begin possible drag selection
  const auto pos = e.GetPosition();
  const auto idx = this->IndexFromLocalX(pos.mX);
  mMouseSelectionAnchor = idx;
  this->SetCaret(idx);

  this->StartMouseCapture();

  // Give focus to this widget so caret is visible and keyboard works
  if (const auto fm = FocusManager::Get()) {
    fm->GivePointerFocus(this);
  }

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

void TextBox::PaintCursor(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  if (!FocusManager::IsWidgetFocused(this)) {
    return;
  }

  if (!mCaretVisible) {
    FUI_ASSERT(SystemSettings::Get().GetCaretBlinkInterval().has_value());
    return;
  }

  const auto& metrics = this->GetMetrics();
  const auto& s = mActiveState;
  const auto midX = metrics.mOffsetX[s.mSelectionStart];

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
    .mOffsetX = {0.0f},
    .mAscent = fontMetrics.mAscent,
    .mDescent = fontMetrics.mDescent,
  };

  const auto& text = mActiveState.mText;
  ret.mOffsetX.reserve(text.size() + 1);

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
  Renderer* renderer,
  const Rect&,
  const Style& style) const {
  const auto& s = mActiveState;
  const auto rect = this->GetContentRect();

  const auto& metrics = this->GetMetrics();
  const auto& font = style.Font().value();

  const auto scrollOffset = metrics.mOffsetX[mContentScrollX];

  Point origin {
    rect.GetLeft() - scrollOffset,
    rect.GetBottom() - metrics.mDescent,
  };

  const auto& color = style.Color().value();

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
  AssertOwnerThread();

  auto& s = mActiveState;
  const auto size = s.mText.size();
  s.mSelectionStart = std::min(start, size);
  s.mSelectionEnd = std::min(end, size);
  // Reset caret blink on movement/selection change
  mCaretVisible = true;
  mLastCaretToggleAt = std::chrono::steady_clock::now();

  if (start == 0 || end == 0) {
    mContentScrollX = 0;
    return;
  }

  // Adjust scroll to keep caret visible
  const auto rect = this->GetContentRect();
  const auto& metrics = this->GetMetrics();
  const auto caretPos = s.mSelectionEnd;
  const auto caretX = metrics.mOffsetX[caretPos];
  const auto scrollX = metrics.mOffsetX[mContentScrollX];
  const auto contentWidth = rect.GetWidth();

  // If caret is off to the left, scroll left
  if (caretX < scrollX) {
    mContentScrollX = caretPos;
  }
  // If caret is off to the right, scroll right
  else if (caretX > scrollX + contentWidth) {
    // Find the rightmost character position where caret would be visible
    for (std::size_t i = mContentScrollX; i <= caretPos; ++i) {
      if (metrics.mOffsetX[caretPos] - metrics.mOffsetX[i] <= contentWidth) {
        mContentScrollX = i;
        break;
      }
    }
  }

  if (mAutomationFlag) {
    return;
  }

  if (const auto sink = mAutomation->GetSink(TS_AS_SEL_CHANGE))
    CheckHResult(sink->OnSelectionChange());
  if (const auto sink = mAutomation->GetSink(TS_AS_LAYOUT_CHANGE))
    CheckHResult(sink->OnLayoutChange(TS_LC_CHANGE, 1));
}

void TextBox::ReplaceSelection(
  const std::string_view newContent,
  const UndoableState::Operation op) {
  this->BeforeOperation(op);
  const auto& s = mActiveState;
  const auto [left, right] = std::minmax(s.mSelectionStart, s.mSelectionEnd);
  this->SetText(
    std::format(
      "{}{}{}", s.mText.substr(0, left), newContent, s.mText.substr(right)));
  this->SetCaret(left + newContent.size());
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

std::size_t TextBox::IndexFromLocalX(const float x) const noexcept {
  // Convert from local widget X to content X (inside padding/border)
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

  std::size_t closestIndex = 0;
  float closestDistance = contentX;// distance from start
  for (auto&& [index, offset]: std::views::enumerate(offsets)) {
    const auto distance = std::abs(offset - contentX);
    if (distance < closestDistance) {
      closestDistance = distance;
      closestIndex = index;
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

  const auto scrollOffset = metrics.mOffsetX[mContentScrollX];
  const auto left
    = contentRect.GetLeft() + metrics.mOffsetX[begin] - scrollOffset;
  auto width = metrics.mOffsetX[end] - metrics.mOffsetX[begin];
  const bool clipped = (width > contentRect.GetWidth()) || (left < 0);
  if (clipped)
    width = contentRect.GetWidth();

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

YGSize TextBox::Measure(
  const YGNodeConstRef node,
  [[maybe_unused]] float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  [[maybe_unused]] float height,
  [[maybe_unused]] YGMeasureMode heightMode) {
  // Getting the parent as `node` is the 'text' child node
  const auto& self = *static_cast<TextBox*>(
    FromYogaNode(YGNodeGetParent(const_cast<YGNodeRef>(node))));

  const auto& metrics = self.GetMetrics();

  return {
    metrics.mOffsetX.back(),
    -metrics.mAscent,
  };
}

}// namespace FredEmmott::GUI::Widgets
