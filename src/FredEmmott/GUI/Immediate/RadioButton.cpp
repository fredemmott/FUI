// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "RadioButton.hpp"

#include "FredEmmott/GUI/Widgets/RadioButton.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

void EndRadioButton() {
  EndWidget<Widgets::RadioButton>();
}

Result<&EndRadioButton, bool> BeginRadioButton(
  const bool isInitiallyChecked,
  const ID id) {
  const auto widget = BeginWidget<Widgets::RadioButton>(id);
  widget->SetIsChecked(isInitiallyChecked);
  return {widget, std::exchange(widget->mClicked, false)};
}

Result<nullptr, bool> RadioButton(
  const bool isInitiallyChecked,
  const std::string_view label,
  const ID id) {
  const auto ret = BeginRadioButton(isInitiallyChecked, id);
  Label(label, ID {0});
  EndRadioButton();
  return ret;
}
}// namespace FredEmmott::GUI::Immediate