// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/StackLayout.hpp>

namespace FredEmmott::GUI::Immediate {

using StackLayoutDirection = Widgets::StackLayout::Direction;

void BeginStackLayout(StackLayoutDirection direction);
void EndStackLayout();

inline void BeginHStackLayout() {
  BeginStackLayout(StackLayoutDirection::Horizontal);
}

inline void BeginVStackLayout() {
  BeginStackLayout(StackLayoutDirection::Vertical);
}

}// namespace FredEmmott::GUI::Immediate