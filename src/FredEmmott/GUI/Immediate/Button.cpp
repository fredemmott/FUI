// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

ButtonResult<&EndButton> BeginButton(bool* clicked, const ID id) {
  using Button = Widgets::Button;
  const auto button = BeginWidget<Button>(id);
  if (clicked) {
    *clicked = button->mClicked.TestAndClear();
  }
  return button;
}

ButtonResult<nullptr, bool> Button(const std::string_view label, const ID id) {
  bool clicked {};
  const auto button = BeginButton(&clicked, id);
  Label(label, ID {0}).Styled({.mFlexGrow = 1});
  EndButton();
  return {button, clicked};
}

}// namespace FredEmmott::GUI::Immediate