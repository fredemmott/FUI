// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "CheckBox.hpp"

#include <FredEmmott/GUI/Immediate/Label.hpp>

namespace FredEmmott::GUI::Immediate {

Result<&EndCheckBox>
BeginCheckBox(bool* pIsChanged, bool* pIsChecked, const ID id) {
  using namespace immediate_detail;
  using Widgets::CheckBox;

  const auto checkbox = BeginWidget<CheckBox>(id);
  const auto isChanged = checkbox->mChanged.TestAndClear();
  if (pIsChanged) {
    *pIsChanged = isChanged;
  }
  if (pIsChecked && isChanged) {
    *pIsChecked = checkbox->IsChecked();
  } else if (pIsChecked) {
    checkbox->SetIsChecked(*pIsChecked);
  }
  return {checkbox};
}

Result<nullptr, bool>
CheckBox(bool* pIsChecked, const std::string_view label, const ID id) {
  bool isChecked {false};
  if (pIsChecked) [[likely]] {
    isChecked = *pIsChecked;
  }
  bool isChanged {false};
  const auto widget = BeginCheckBox(&isChanged, &isChecked, id);

  if (pIsChecked) [[likely]] {
    *pIsChecked = isChecked;
  }

  Label(label, ID {0});

  EndCheckBox();
  return {widget, isChanged};
}

}// namespace FredEmmott::GUI::Immediate
