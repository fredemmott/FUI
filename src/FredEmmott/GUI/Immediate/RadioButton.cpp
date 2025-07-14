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
  const auto changed = std::exchange(widget->mChanged, false);
  if (!changed) {
    widget->SetIsChecked(isInitiallyChecked);
  }
  return {
    widget,
    changed && widget->IsChecked(),
  };
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