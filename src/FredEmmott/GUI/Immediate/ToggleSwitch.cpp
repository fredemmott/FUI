// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitch.hpp"

#include <FredEmmott/GUI/Immediate/Label.hpp>

namespace FredEmmott::GUI::Immediate {

void BeginToggleSwitch(
  bool* pIsChanged,
  bool* pIsOn,
  const Widgets::WidgetStyles& styles) {
  using namespace immediate_detail;
  using Widgets::ToggleSwitch;

  BeginWidget<ToggleSwitch> {}(styles);

  auto toggle = GetCurrentParentNode<ToggleSwitch>();
  const auto isChanged = toggle->mChanged.TestAndClear();
  if (pIsChanged) {
    *pIsChanged = isChanged;
  }
  if (pIsOn && isChanged) {
    *pIsOn = toggle->IsOn();
  } else if (pIsOn) {
    toggle->SetIsOn(*pIsOn);
  }
}

bool ToggleSwitch(
  bool* pIsOn,
  const Widgets::WidgetStyles& styles,
  const FormattedString& offText,
  const FormattedString& onText) {
  bool isOn {pIsOn ? *pIsOn : false};
  bool isChanged {false};
  BeginToggleSwitch(&isChanged, &isOn, styles);

  Label("{}##Label", std::string_view {isOn ? onText : offText});

  if (pIsOn) {
    *pIsOn = isOn;
  }

  EndToggleSwitch();
  return isChanged;
}

}// namespace FredEmmott::GUI::Immediate
