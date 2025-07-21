// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/StackPanel.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndStackPanel() {
  immediate_detail::EndWidget<Widgets::StackPanel>();
}

inline void EndVStackPanel() {
  EndStackPanel();
}

inline void EndHStackPanel() {
  EndStackPanel();
}

inline Result<&EndHStackPanel> BeginHStackPanel(
  ID id = ID {std::source_location::current()}) {
  using Widgets::StackPanel;
  return {
    immediate_detail::BeginWidget<StackPanel>(id, Orientation::Horizontal)};
}

inline Result<&EndVStackPanel> BeginVStackPanel(
  ID id = ID {std::source_location::current()}) {
  using Widgets::StackPanel;
  return {immediate_detail::BeginWidget<StackPanel>(id, Orientation::Vertical)};
}

}// namespace FredEmmott::GUI::Immediate