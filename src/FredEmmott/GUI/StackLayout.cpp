// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackLayout.hpp"

namespace FredEmmott::GUI {

StackLayout::StackLayout(std::size_t id, const Options&, Direction direction)
  : Layout(id) {
  YGNodeStyleSetFlexDirection(
    this->GetLayoutNode(),
    direction == Direction::Horizontal ? YGFlexDirectionRow
                                       : YGFlexDirectionColumn);
}

}// namespace FredEmmott::GUI