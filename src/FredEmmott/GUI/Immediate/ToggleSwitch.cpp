// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitch.hpp"

#include <FredEmmott/GUI/Immediate/Label.hpp>

namespace FredEmmott::GUI::Immediate {

void BeginToggleSwitch(bool* pIsChanged, bool* pIsOn, const ID id) {
  using namespace immediate_detail;
  using Widgets::ToggleSwitch;

  BeginWidget<ToggleSwitch>(id);
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
  std::string_view onText,
  std::string_view offText,
  const ID id) {
  bool isOn {false};
  if (pIsOn) [[likely]] {
    isOn = *pIsOn;
  }
  bool isChanged {false};
  BeginToggleSwitch(&isChanged, &isOn, id);

  if (pIsOn) [[likely]] {
    *pIsOn = isOn;
  }

  Label(isOn ? onText : offText, ID {0});

  EndToggleSwitch();
  return isChanged;
}

}// namespace FredEmmott::GUI::Immediate
