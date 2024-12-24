// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/StackLayout.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

namespace FredEmmott::GUI::Immediate {

inline void BeginHStackLayout() {
  using Widgets::StackLayout;
  immediate_detail::
    BeginWidget<StackLayout, nullptr, StackLayout::Direction::Horizontal> {}();
}

inline void BeginVStackLayout() {
  using Widgets::StackLayout;
  immediate_detail::
    BeginWidget<StackLayout, nullptr, StackLayout::Direction::Vertical> {}();
}

inline void EndStackLayout() {
  immediate_detail::EndWidget();
}

}// namespace FredEmmott::GUI::Immediate