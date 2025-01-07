// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackPanel.hpp"

namespace FredEmmott::GUI::Widgets {

StackPanel::StackPanel(std::size_t id, Direction direction)
  : Widget(id), mDirection(direction) {
}

WidgetStyles StackPanel::GetBuiltInStyles() const {
  return {
    .mBase = {
      .mFlexDirection = (mDirection == Direction::Horizontal) ? YGFlexDirectionRow : YGFlexDirectionColumn,
      .mGap = Spacing * 4,
    },
  };
}

}// namespace FredEmmott::GUI::Widgets