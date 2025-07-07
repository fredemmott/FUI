// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndDisabled() {
  immediate_detail::EndWidget<Widgets::Widget>();
}

inline void EndEnabled() {
  EndDisabled();
}

Result<&EndDisabled, void, immediate_detail::WidgetlessResultMixin>
BeginDisabled(
  bool isDisabled = true,
  ID id = ID {std::source_location::current()});

inline Result<&EndEnabled, void, immediate_detail::WidgetlessResultMixin>
BeginEnabled(
  const bool isEnabled = true,
  const ID id = ID {std::source_location::current()}) {
  BeginDisabled(!isEnabled, id);
  return {};
}

}// namespace FredEmmott::GUI::Immediate