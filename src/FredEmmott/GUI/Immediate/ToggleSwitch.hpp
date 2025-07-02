// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ToggleSwitch.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndToggleSwitch() {
  immediate_detail::EndWidget<Widgets::ToggleSwitch>();
}

Result<&EndToggleSwitch> BeginToggleSwitch(
  bool* isChanged,
  bool* isOn,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
Result<nullptr, bool> ToggleSwitch(
  bool* isOn,
  std::string_view onText = "On",
  std::string_view offText = "Off",
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate