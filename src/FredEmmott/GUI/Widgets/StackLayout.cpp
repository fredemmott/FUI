// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackLayout.hpp"

namespace FredEmmott::GUI::Widgets {

StackLayout::StackLayout(std::size_t id, Direction direction)
  : Widget(id), mDirection(direction) {
}

WidgetStyles StackLayout::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mDefault = {
      .mFlexDirection = (mDirection == Direction::Horizontal) ? YGFlexDirectionRow : YGFlexDirectionColumn,
      .mGap = Spacing,
    },
  };
  return ret;
}

}// namespace FredEmmott::GUI::Widgets