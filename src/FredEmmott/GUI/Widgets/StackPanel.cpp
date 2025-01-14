// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackPanel.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
const auto HorizontalStyleClass = Style::Class::Make("HorizontalStackPanel");
const auto VerticalStyleClass = Style::Class::Make("VerticalStackPanel");
}// namespace

StackPanel::StackPanel(std::size_t id, Orientation orientation)
  : Widget(
      id,
      {orientation == Orientation::Horizontal ? HorizontalStyleClass
                                              : VerticalStyleClass}),
    mOrientation(orientation) {}

WidgetStyles StackPanel::GetBuiltInStyles() const {
  return {
    .mBase = {
      .mFlexGrow = 1,
      .mGap = Spacing * 4,
      .mAnd = {
        {
          HorizontalStyleClass,
          Style {
            .mFlexDirection = YGFlexDirectionRow,
          },
        },
        {
          VerticalStyleClass,
          Style {
            .mFlexDirection = YGFlexDirectionColumn,
          },
        }
      },
    },
  };
}

}// namespace FredEmmott::GUI::Widgets