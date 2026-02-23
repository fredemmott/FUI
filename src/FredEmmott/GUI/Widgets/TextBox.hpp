// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <chrono>
#include <mutex>
#include <vector>

#include "Focusable.hpp"
#include "FredEmmott/GUI/detail/AutomationActivityFlag.hpp"
#include "FredEmmott/GUI/detail/icu.hpp"
#include "Label.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI {
class Window;
namespace win32_detail {
class TextStoreACP;
}
}// namespace FredEmmott::GUI
namespace FredEmmott::GUI::Widgets {

class TextBox : public Widget, public IFocusable {
 public:
  enum class DeleteDirection {
    DeleteLeft,
    DeleteRight,
  };
  struct BoundingBox {
    Rect mRect;
    bool mClipped {false};
  };
  explicit TextBox(std::size_t id);
  ~TextBox() override;

  [[nodiscard]]
  bool ConsumeWasChanged() noexcept {
    return std::exchange(mWasChanged, false);
  }

  void SetText(std::string_view);

  FrameRateRequirement GetFrameRateRequirement() const noexcept override;
  [[nodiscard]] std::string_view GetText() const noexcept {
    return mActiveState.mText;
  }

  // Accessors used by TSF and automation
  auto GetAutomationActivityGuard() {
    return std::unique_lock {mAutomationFlag};
  }
  [[nodiscard]]
  Rect GetContentRect() const noexcept;
  void DeleteSelection(DeleteDirection ifSelectionEmpty);

  [[nodiscard]] std::wstring_view GetTextW() const noexcept;
  void SetTextW(std::wstring_view);

  [[nodiscard]] std::pair<std::size_t, std::size_t> GetSelectionW() const;
  void SetSelectionW(std::size_t begin, std::size_t end);

  [[nodiscard]]
  BoundingBox GetTextBoundingBoxW(std::size_t begin, std::size_t end)
    const noexcept;

 protected:
  void Tick(const std::chrono::steady_clock::time_point& now) override;
  EventHandlerResult OnTextInput(const TextInputEvent&) override;
  EventHandlerResult OnKeyPress(const KeyPressEvent&) override;
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;
  [[nodiscard]] EventHandlerResult OnMouseButtonPress(
    const MouseEvent&) override;
  [[nodiscard]] EventHandlerResult OnMouseMove(const MouseEvent&) override;
  [[nodiscard]] EventHandlerResult OnMouseButtonRelease(
    const MouseEvent&) override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  // Data needed by accessibility or international input
  struct Automation;
  std::unique_ptr<Automation> mAutomation;
  detail::AutomationActivityFlag mAutomationFlag;
#ifndef NDEBUG
  std::thread::id mOwnerThread {std::this_thread::get_id()};
  void AssertOwnerThread() const {
    FUI_ASSERT(mOwnerThread == std::this_thread::get_id());
  }
#else
  void AssertOwnerThread() const {}
#endif
  bool mWasChanged {false};

  struct TextMetrics {
    // Cumulative width at each byte index into the UTF-8 string.
    // mWidthToIndex[i] is the width of text.substr(0, i).
    // Size is text.size() + 1 with mWidthToIndex[0] == 0.
    std::vector<float> mOffsetX;
    float mAscent {};
    float mDescent {};
  };
  struct UndoableState {
    enum class Operation {
      None,
      Cut,
      Paste,
      Typing,
      DeleteLeft,
      DeleteRight,
    };
    Operation mOperation {Operation::None};

    std::string mText;
    std::size_t mSelectionStart {};
    std::size_t mSelectionEnd {};
  };
  struct Caches {
    unique_ptr<UText, &utext_close> mUText;
    std::wstring mWideText;

    unique_ptr<UBreakIterator, &ubrk_close> mGraphemeIterator;
    unique_ptr<UBreakIterator, &ubrk_close> mWordIterator;
    std::optional<TextMetrics> mTextMetrics;
  };

  UndoableState mActiveState {};
  UndoableState mUndoState {};

  mutable Caches mCaches {};

  bool mIsFocused {false};
  std::optional<std::size_t> mMouseSelectionAnchor;

  // Caret blinking state
  bool mCaretVisible {true};
  std::chrono::steady_clock::time_point mLastCaretToggleAt {};

  // Horizontal scroll position (number of characters hidden off to the left)
  std::size_t mContentScrollX {0};

  void BeforeOperation(UndoableState::Operation);

  const TextMetrics& GetMetrics() const;

  void PaintCursor(Renderer*, const Rect&, const Style&) const;

  UText* GetUText() const noexcept;
  UBreakIterator* GetGraphemeIterator() const noexcept;
  UBreakIterator* GetWordIterator() const noexcept;

  std::size_t GetPreviousWordBoundary() const noexcept;
  std::size_t GetNextWordBoundary() const noexcept;

  // Convert an X position in local widget coordinates to a text caret index
  std::size_t IndexFromLocalX(float x) const noexcept;

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);

  [[nodiscard]] std::pair<std::size_t, std::size_t> GetSelection()
    const noexcept {
    return {mActiveState.mSelectionStart, mActiveState.mSelectionEnd};
  }
  [[nodiscard]]
  BoundingBox GetTextBoundingBox(std::size_t begin, std::size_t end)
    const noexcept;
  void SetSelection(std::size_t start, std::size_t end);
  void SetCaret(const std::size_t pos) {
    this->SetSelection(pos, pos);
  }
  void ReplaceSelection(std::string_view, UndoableState::Operation);
};

}// namespace FredEmmott::GUI::Widgets