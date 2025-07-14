// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackPanel.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
const auto HorizontalStyleClass = StyleClass::Make("HorizontalStackPanel");
const auto VerticalStyleClass = StyleClass::Make("VerticalStackPanel");

const auto BaseStyles = Style().FlexGrow(0).Gap(16);
const auto HorizontalStyles
  = BaseStyles + Style().FlexDirection(YGFlexDirectionRow);
const auto VerticalStyles
  = BaseStyles + Style().FlexDirection(YGFlexDirectionColumn);
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