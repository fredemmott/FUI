// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ToggleSwitch.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

void BeginToggleSwitch(
  bool* isChanged,
  bool* isOn,
  ID id = ID {std::source_location::current()});
inline void EndToggleSwitch() {
  immediate_detail::EndWidget<Widgets::ToggleSwitch>();
}

[[nodiscard]]
bool ToggleSwitch(
  bool* isOn,
  std::string_view onText = "On",
  std::string_view offText = "Off",
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate