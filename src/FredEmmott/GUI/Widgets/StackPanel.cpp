// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackPanel.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass HorizontalStyleClass {"HorizontalStackPanel"};
constexpr LiteralStyleClass VerticalStyleClass {"VerticalStackPanel"};

auto BaseStyles() {
  return Style().FlexGrow(0).Gap(16);
}

auto& HorizontalStyles() {
  static const ImmutableStyle ret {
    BaseStyles() + Style().FlexDirection(YGFlexDirectionRow),
  };
  return ret;
}

auto& VerticalStyles() {
  static const ImmutableStyle ret {
    BaseStyles() + Style().FlexDirection(YGFlexDirectionColumn),
  };
  return ret;
}

}// namespace

StackPanel::StackPanel(const Orientation orientation)
  : Widget(
      LiteralStyleClass {"StackPanel"},
      (orientation == Orientation::Horizontal) ? HorizontalStyles()
                                               : VerticalStyles(),
      {orientation == Orientation::Horizontal ? *HorizontalStyleClass
                                              : *VerticalStyleClass}) {}

StackPanel::~StackPanel() = default;

}// namespace FredEmmott::GUI::Widgets