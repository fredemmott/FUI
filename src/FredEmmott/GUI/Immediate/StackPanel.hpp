// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/StackPanel.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

inline void BeginHStackPanel() {
  using Widgets::StackPanel;
  immediate_detail::
    BeginWidget<StackPanel, nullptr, StackPanel::Direction::Horizontal> {}();
}

inline void BeginVStackPanel() {
  using Widgets::StackPanel;
  immediate_detail::
    BeginWidget<StackPanel, nullptr, StackPanel::Direction::Vertical> {}();
}

inline void EndStackPanel() {
  immediate_detail::EndWidget<Widgets::StackPanel>();
}

}// namespace FredEmmott::GUI::Immediate