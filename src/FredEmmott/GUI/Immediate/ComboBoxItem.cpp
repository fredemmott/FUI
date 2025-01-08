// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include "Button.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginComboBoxItem(bool* selected, ID id) {
  BeginButton(selected, id);
}

void EndComboBoxItem() {
  EndButton();
}

bool ComboBoxItem(bool initiallySelected, std::string_view label, ID id) {
  bool selected = initiallySelected;
  BeginComboBoxItem(&selected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return selected;
}

}// namespace FredEmmott::GUI::Immediate