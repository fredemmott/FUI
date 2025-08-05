// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.hpp"

#include "FredEmmott/GUI/FocusManager.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"

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

  static const ImmutableStyle CaretStyle {
    Style().Width(1).BackgroundColor(Colors::Red),
  };
  static const ImmutableStyle SelectionStyle {
    Style().BackgroundColor(Colors::Blue).Color(Colors::White)};

  this->SetChildren({
    mBeforeSelection = new Label(0),
    mCaret = new Widget(0, CaretStyle),
    mSelection = new Label(0),
    mAfterSelection = new Label(0),
  });

  this->SetText("Hello, world");
}

TextBox::~TextBox() = default;

void TextBox::SetText(const std::string_view text) {
  if (text == mText) {
    return;
  }

  mText = std::string {text};
  mSelectionStart = 0;
  mSelectionEnd = 0;
  mBeforeSelection->SetText({});
  mSelection->SetText({});
  mAfterSelection->SetText(mText);

  // TODO: notify IME
}

void TextBox::Tick() {
  const auto isFocused = FocusManager::Get()->IsWidgetFocused(this);
  if (isFocused == mIsFocused) {
    return;
  }
  mIsFocused = isFocused;

  // TODO: notify IME
}

}// namespace FredEmmott::GUI::Widgets
