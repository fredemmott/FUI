// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "TitleBar.handwritten.hpp"

namespace FredEmmott::GUI::StaticTheme::TitleBar {
const ImmutableStyle& DefaultTitleBarStyle() {
  static const ImmutableStyle ret {
    Style()
      .FlexDirection(FlexDirection::Row)
      .JustifyContent(YGJustifyFlexEnd)
      .MinHeight(TitleBarCompactHeight)
      .MaxHeight(TitleBarExpandedHeight)
      .And(
        TitleBarInactiveWindowStyleClass,
        Style().Opacity(TitleBarDeactivatedOpacity))};
  return ret;
}
const ImmutableStyle& WindowMinimizeMaximizeButtonStyle() {
  using namespace StaticTheme::Generic;
  using namespace PseudoClasses;

  // Only used on mouse-out, not on mouse-in
  //
  // Behavior based on Microsoft Terminal
  static constexpr auto BackgroundColorAnimation
    = LinearStyleTransition(std::chrono::milliseconds(150));
  static constexpr auto ForegroundColorAnimation
    = LinearStyleTransition(std::chrono::milliseconds(100));

  static const ImmutableStyle ret {
    Style()
      .AlignSelf(Align::Stretch)
      .AlignItems(Align::Center)
      .AspectRatio(1.0f)
      .JustifyContent(YGJustifyCenter)
      .BackgroundColor(WindowCaptionButtonBackground, BackgroundColorAnimation)
      .Color(WindowCaptionButtonStroke, ForegroundColorAnimation)
      .Font(
        SystemFont::ResolveGlyphFont(SystemFont::Body)
          .WithSize(10)
          .WithWeight(FontWeight::Normal),
        !important)
      // TODO: disabled
      .And(
        Hover,
        Style()
          .BackgroundColor(
            WindowCaptionButtonBackgroundPointerOver, InstantStyleTransition)
          .Color(WindowCaptionButtonStrokePointerOver, InstantStyleTransition))
      .And(
        Active,
        Style()
          .BackgroundColor(
            WindowCaptionButtonBackgroundPressed, InstantStyleTransition)
          .Color(WindowCaptionButtonStrokePressed, InstantStyleTransition))};
  return ret;
}
const ImmutableStyle& WindowCloseButtonStyle() {
  using namespace StaticTheme::Generic;
  using namespace PseudoClasses;
  static const ImmutableStyle ret {
    WindowMinimizeMaximizeButtonStyle()
    + Style()
        .And(
          Hover,
          Style()
            .BackgroundColor(
              CloseButtonBackgroundPointerOver, InstantStyleTransition)
            .Color(CloseButtonStrokePointerOver, InstantStyleTransition))
        .And(
          Active,
          Style()
            .BackgroundColor(
              CloseButtonBackgroundPressed, InstantStyleTransition)
            .Color(CloseButtonStrokePressed, InstantStyleTransition))};
  return ret;
}
const ImmutableStyle& TitleBarContentContainerStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignItems(Align::Center)
      .FlexDirection(FlexDirection::Row)
      .FlexGrow(1)
      .PaddingLeft(TitleBarLeftPaddingWidth)
      .PaddingRight(TitleBarRightPaddingWidth),
  };
  return ret;
}
const ImmutableStyle& TitleBarTitleStyle() {
  static const ImmutableStyle ret {
    Style()
      .Color(TitleBarForegroundBrush)
      .Font(SystemFont::Caption)
      .Margin(TitleBarTitleMargin)
      .MinWidth(TitleBarTitleMinWidth)
      .And(
        TitleBarInactiveWindowStyleClass,
        Style().Color(TitleBarDeactivatedForegroundBrush)),
  };
  return ret;
}
const ImmutableStyle& TitleBarSubtitleStyle() {
  static const ImmutableStyle ret {
    Style()
      .Color(TitleBarSubtitleForegroundBrush)
      .Font(SystemFont::Caption)
      .Margin(TitleBarSubtitleMargin)
      .MinWidth(TitleBarSubtitleMinWidth)
      .And(
        TitleBarInactiveWindowStyleClass,
        Style().Color(TitleBarSubtitleDeactivatedForegroundBrush)),
  };
  return ret;
}
const ImmutableStyle& TitleBarIconStyle() {
  static const ImmutableStyle ret {
    Style()
      .AspectRatio(1.0f)
      .FlexGrow(1)
      .MarginLeft(TitleBarIconMargin.GetLeft() + TitleBarLeftHeaderPaddingWidth)
      .MarginTop(TitleBarIconMargin.GetTop())
      .MarginRight(TitleBarIconMargin.GetRight())
      .MarginBottom(TitleBarIconMargin.GetBottom())
      .MaxWidth(TitleBarIconMaxWidth)
      .MaxHeight(TitleBarIconMaxHeight)
      .Height(TitleBarIconMaxHeight)
      .Width(TitleBarIconMaxWidth),
  };
  return ret;
}
}// namespace FredEmmott::GUI::StaticTheme::TitleBar