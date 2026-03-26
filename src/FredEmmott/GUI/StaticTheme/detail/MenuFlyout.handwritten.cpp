// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <FredEmmott/GUI/StaticTheme/MenuFlyout.hpp>

namespace FredEmmott::GUI::StaticTheme::MenuFlyout {

static const ImmutableStyle& DefaultMenuFlyoutItemStyle() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::Generic;
  using namespace PseudoClasses;
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(MenuFlyoutItemBackground)
      // Yep, I'd expect BorderBrush, but it's definitely BackgroundBrush in the
      // XAML
      .BorderColor(MenuFlyoutItemBackgroundBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(MenuFlyoutItemBorderThickness)
      .Color(MenuFlyoutItemForeground)
      .Padding(MenuFlyoutItemThemePadding)
      .Margin(MenuFlyoutItemMargin)
      .Font(SystemFont::Resolve(ControlContentThemeFontSize))
      .And(
        Disabled,
        Style()
          .BackgroundColor(MenuFlyoutItemBackgroundDisabled)
          .Color(MenuFlyoutItemForegroundDisabled))
      .And(
        Hover,
        Style()
          .BackgroundColor(MenuFlyoutItemBackgroundPointerOver)
          .Color(MenuFlyoutItemForegroundPointerOver))
      .And(
        Active,
        Style()
          .BackgroundColor(MenuFlyoutItemBackgroundPressed)
          .Color(MenuFlyoutItemForegroundPressed))

  };
  return ret;
}

const ImmutableStyle& MenuFlyoutItemStyle() {
  return DefaultMenuFlyoutItemStyle();
}

const ImmutableStyle& MenuFlyoutStyle() {
  static ImmutableStyle ret {
    Style()
      .BackgroundColor(Common::AcrylicBackgroundFillColorDefaultBrush)
      .BorderColor(MenuFlyoutPresenterBorderBrush)
      .BorderWidth(MenuFlyoutPresenterBorderThemeThickness)
      .BorderRadius(Common::OverlayCornerRadius)
      .FlexDirection(FlexDirection::Column)
      .MinHeight(MenuFlyoutThemeMinHeight)
      .MinWidth(Generic::FlyoutThemeMinWidth)
      .Padding(MenuFlyoutPresenterThemePadding),
  };
  return ret;
}

const ImmutableStyle& MenuFlyoutSeparatorStyle() {
  static ImmutableStyle ret {
    Style()
      .BackgroundColor(MenuFlyoutSeparatorBackground)
      // The original XAML also uses this 'padding' as margin
      .Margin(MenuFlyoutSeparatorThemePadding)
      .Height(MenuFlyoutSeparatorHeight),
  };
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::MenuFlyout