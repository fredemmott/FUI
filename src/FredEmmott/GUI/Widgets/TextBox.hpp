// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "FredEmmott/GUI/detail/icu.hpp"
#include "Label.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI {
class Window;
}
namespace FredEmmott::GUI::Widgets {

class TextBox : public Widget, public IFocusable {
 public:
  explicit TextBox(std::size_t id);
  ~TextBox() override;

  void SetText(std::string_view);

 protected:
  void Tick() override;
  EventHandlerResult OnTextInput(const TextInputEvent&) override;
  EventHandlerResult OnKeyPress(const KeyPressEvent&) override;
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  enum DeleteDirection {
    DeleteLeft,
    DeleteRight,
  };
  struct TextMetrics {
    float mWidthBeforeSelection {};
    float mWidthOfSelection {};
    float mWidthAfterSelection {};
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
    unique_ptr<UBreakIterator, &ubrk_close> mGraphemeIterator;
    unique_ptr<UBreakIterator, &ubrk_close> mWordIterator;
    std::optional<TextMetrics> mTextMetrics;
  };

  Window* mWindow {};

  UndoableState mActiveState {};
  UndoableState mUndoState {};

  mutable Caches mCaches {};

  bool mIsFocused {false};

  void BeforeOperation(UndoableState::Operation);

  const TextMetrics& GetMetrics() const;

  void PaintCursor(Renderer*, const Rect&, const Style&) const;

  void SetCaret(const std::size_t pos) {
    this->SetSelection(pos, pos);
  }
  void SetSelection(std::size_t start, std::size_t end);

  UText* GetUText() const noexcept;
  UBreakIterator* GetGraphemeIterator() const noexcept;
  UBreakIterator* GetWordIterator() const noexcept;

  std::size_t GetPreviousWordBoundary() const noexcept;
  std::size_t GetNextWordBoundary() const noexcept;

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);

  void DeleteSelection(DeleteDirection ifSelectionEmpty);
};

}// namespace FredEmmott::GUI::Widgets