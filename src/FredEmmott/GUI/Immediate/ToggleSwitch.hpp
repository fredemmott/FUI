// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/ToggleSwitch.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

template <void (*TEndWidget)() = nullptr, class TValue = void>
using ToggleSwitchResult
  = Result<TEndWidget, TValue, immediate_detail::CaptionResultMixin>;

inline void EndToggleSwitch() {
  immediate_detail::EndWidget<Widgets::ToggleSwitch>();
}

ToggleSwitchResult<&EndToggleSwitch> BeginToggleSwitch(
  bool* isChanged,
  bool* isOn,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
ToggleSwitchResult<nullptr, bool> ToggleSwitch(
  bool* isOn,
  std::string_view onText = "On",
  std::string_view offText = "Off",
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate