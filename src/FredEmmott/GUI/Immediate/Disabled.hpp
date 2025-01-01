// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginDisabled(
  bool isDisabled,
  ID id = ID {std::source_location::current()});

inline void EndDisabled() {
  immediate_detail::EndWidget<Widgets::Widget>();
}

inline void BeginEnabled(
  const bool isEnabled = true,
  const ID id = ID {std::source_location::current()}) {
  BeginDisabled(!isEnabled, id);
}

inline void EndEnabled() {
  EndDisabled();
}

}// namespace FredEmmott::GUI::Immediate