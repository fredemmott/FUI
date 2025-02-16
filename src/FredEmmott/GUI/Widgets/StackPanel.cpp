// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackPanel.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
const auto HorizontalStyleClass = StyleClass::Make("HorizontalStackPanel");
const auto VerticalStyleClass = StyleClass::Make("VerticalStackPanel");

const Style BaseStyles = {
  .mFlexGrow = 1,
  .mGap = 16,
};
const auto HorizontalStyles = BaseStyles
  + Style {
    .mFlexDirection = YGFlexDirectionRow,
  };
const auto VerticalStyles = BaseStyles
  + Style {
    .mFlexDirection = YGFlexDirectionColumn,
  };
}// namespace

StackPanel::StackPanel(std::size_t id, Orientation orientation)
  : Widget(
      id,
      {orientation == Orientation::Horizontal ? HorizontalStyleClass
                                              : VerticalStyleClass}),
    mOrientation(orientation) {}

Style StackPanel::GetBuiltInStyles() const {
  return {
    (mOrientation == Orientation::Horizontal) ? HorizontalStyles
                                              : VerticalStyles};
}

}// namespace FredEmmott::GUI::Widgets