// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TitleBar.handwritten.hpp"

namespace FredEmmott::GUI::StaticTheme::TitleBar {
const ImmutableStyle& DefaultTitleBarStyle() {
  static const ImmutableStyle ret {
    Style()
      .FlexDirection(YGFlexDirectionRow)
      .JustifyContent(YGJustifyFlexEnd)
      .Height(TitleBarCompactHeight),
  };
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
      .AlignSelf(YGAlignStretch)
      .AlignItems(YGAlignCenter)
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
      .AlignItems(YGAlignCenter)
      .FlexDirection(YGFlexDirectionRow)
      .FlexGrow(1)
      .PaddingLeft(
        // Before any contols, e.g. back button, which we don't currently have
        TitleBarLeftPaddingWidth
        // Before the icon
        + TitleBarLeftHeaderPaddingWidth)
      .PaddingRight(TitleBarRightPaddingWidth),
  };
  return ret;
}
const ImmutableStyle& TitleBarTitleStyle() {
  // TODO: deactivated brushes
  static const ImmutableStyle ret {
    Style()
      .Font(SystemFont::Caption)
      .MarginLeft(TitleBarTitleMarginLeft)
      .MarginTop(TitleBarTitleMarginTop)
      .MarginRight(TitleBarTitleMarginRight)
      .MarginBottom(TitleBarTitleMarginBottom)
      .MinWidth(TitleBarTitleMinWidth),
  };
  return ret;
}
const ImmutableStyle& TitleBarSubtitleStyle() {
  // TODO: deactivated brushes
  static const ImmutableStyle ret {
    Style()
      .Color(TitleBarSubtitleForegroundBrush)
      .Font(SystemFont::Caption)
      .MarginLeft(TitleBarSubtitleMarginLeft)
      .MarginTop(TitleBarSubtitleMarginTop)
      .MarginRight(TitleBarSubtitleMarginRight)
      .MarginBottom(TitleBarSubtitleMarginBottom)
      .MinWidth(TitleBarSubtitleMinWidth),
  };
  return ret;
}
const ImmutableStyle& TitleBarIconStyle() {
  static const ImmutableStyle ret {
    Style()
      .AspectRatio(1.0f)
      .FlexGrow(1)
      .MarginLeft(TitleBarIconMarginLeft)
      .MarginTop(TitleBarIconMarginTop)
      .MarginRight(TitleBarIconMarginRight)
      .MarginBottom(TitleBarIconMarginBottom)
      .MaxWidth(TitleBarIconMaxWidth)
      .MaxHeight(TitleBarIconMaxHeight),
  };
  return ret;
}
}// namespace FredEmmott::GUI::StaticTheme::TitleBar