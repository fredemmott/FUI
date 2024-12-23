// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackLayout.hpp"

namespace FredEmmott::GUI::Widgets {

StackLayout::StackLayout(std::size_t id, const Options&, Direction direction)
  : Layout(id) {
  const auto layout = this->GetLayoutNode();
  YGNodeStyleSetFlexDirection(
    layout,
    direction == Direction::Horizontal ? YGFlexDirectionRow
                                       : YGFlexDirectionColumn);
  YGNodeStyleSetGap(layout, YGGutterAll, Spacing);
}

}// namespace FredEmmott::GUI::Widgets