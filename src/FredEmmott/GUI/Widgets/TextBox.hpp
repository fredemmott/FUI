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
  struct TextMetrics {
    float mWidthBeforeSelection {};
    float mWidthOfSelection {};
    float mWidthAfterSelection {};
    float mAscent {};
    float mDescent {};
  };
  Window* mWindow {};

  std::string mText;
  unique_ptr<UText, &utext_close> mUText;
  unique_ptr<UBreakIterator, &ubrk_close> mGraphemeIterator;

  std::size_t mSelectionStart {};
  std::size_t mSelectionEnd {};
  bool mIsFocused {false};

  mutable std::optional<TextMetrics> mTextMetrics;

  const TextMetrics& GetMetrics() const;

  void PaintCursor(Renderer*, const Rect&, const Style&) const;

  void SetCaret(const std::size_t pos) {
    this->SetSelection(pos, pos);
  }
  void SetSelection(std::size_t start, std::size_t end);

  UText* GetUText() noexcept;
  UBreakIterator* GetGraphemeIterator() noexcept;

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};

}// namespace FredEmmott::GUI::Widgets