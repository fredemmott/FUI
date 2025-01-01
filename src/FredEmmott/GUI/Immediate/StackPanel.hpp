// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/StackPanel.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

inline void BeginHStackPanel(ID id = ID {std::source_location::current()}) {
  using Widgets::StackPanel;
  immediate_detail::BeginWidget<StackPanel, StackPanel::Direction::Horizontal>(
    id);
}

inline void BeginVStackPanel(ID id = ID {std::source_location::current()}) {
  using Widgets::StackPanel;
  immediate_detail::BeginWidget<StackPanel, StackPanel::Direction::Vertical>(
    id);
}

inline void EndStackPanel() {
  immediate_detail::EndWidget<Widgets::StackPanel>();
}

}// namespace FredEmmott::GUI::Immediate