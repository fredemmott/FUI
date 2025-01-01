// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

void BeginButton(bool* clicked, const ID id) {
  using Button = Widgets::Button;
  BeginWidget<Button>(id);
  if (clicked) {
    *clicked = GetCurrentParentNode<Button>()->mClicked.TestAndClear();
  }
}

bool Button(const std::string_view label, const ID id) {
  bool clicked {};
  BeginButton(&clicked, id);
  Label(label, ID {0});
  EndButton();
  return clicked;
}

}// namespace FredEmmott::GUI::Immediate