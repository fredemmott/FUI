// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/StaticTheme/NavigationView.hpp>
#include <FredEmmott/GUI/StaticTheme/SplitView.hpp>

namespace FredEmmott::GUI::StaticTheme::NavigationView {

const ImmutableStyle& NavigationViewStyle() {
  using namespace StaticTheme::NavigationView;
  static const ImmutableStyle ret {
    Style()
      .FlexDirection(FlexDirection::Row)
      .AlignItems(Align::Stretch)
      .AlignSelf(Align::Stretch)
      .FlexGrow(1)
      .FlexShrink(1),
  };
  return ret;
}

const ImmutableStyle& NavigationViewPaneStyle() {
  using namespace StaticTheme::SplitView;
  // Taken from SplitView_themeresources.xaml
  static constexpr std::array<float, 4> WidthAnimationKeySpline {
    0.0f,
    0.35f,
    0.15f,
    1.0f,
  };
  static constexpr auto ExpandAnimation = CubicBezierStyleTransition(
    SplitViewPaneAnimationOpenDuration, WidthAnimationKeySpline);
  static constexpr auto CollapseAnimation = CubicBezierStyleTransition(
    SplitViewPaneAnimationCloseDuration, WidthAnimationKeySpline);

  static const ImmutableStyle ret {
    Style()
      // TODO: set this when *not* bleeding into title bar
      // .BackgroundColor(NavigationViewDefaultPaneBackground)
      .AlignItems(Align::Center)
      .FlexDirection(FlexDirection::Column)
      .Width(NavigationViewCompactPaneLength, CollapseAnimation)
      .And(
        NavigationViewPaneExpandedStyleClass,
        Style()
          .AlignItems(Align::FlexStart)
          .BackgroundColor(NavigationViewExpandedPaneBackground)
          .Width(SplitViewOpenPaneThemeLength, ExpandAnimation)
          .Descendants(
            NavigationViewItemLabelStyleClass, Style().Display(Display::Flex)))
      .Descendants(
        NavigationViewItemLabelStyleClass, Style().Display(Display::None))};
  return ret;
}
const ImmutableStyle& NavigationViewPaneHeaderStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignItems(Align::FlexStart)
      .AlignSelf(Align::Stretch)
      .FlexDirection(FlexDirection::Row)
      .Height(NavigationViewPaneHeaderRowMinHeight)
      .Margin(NavigationViewButtonHolderGridMargin)};
  return ret;
}

const ImmutableStyle& NavigationViewItemsRootStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignSelf(Align::Stretch)
      .FlexDirection(FlexDirection::Column)
      .FlexGrow(1)
      .AlignItems(Align::FlexStart),
  };
  return ret;
}
const ImmutableStyle& NavigationViewFooterItemsRootStyle() {
  static const ImmutableStyle ret {
    NavigationViewItemsRootStyle() + Style().FlexGrow(0).MarginBottom(2)};
  return ret;
}

const ImmutableStyle& NavigationViewContentOuterStyle() {
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(NavigationViewContentBackground)
      .BorderColor(NavigationViewContentGridBorderBrush)
      .BorderRadius(NavigationViewContentGridCornerRadius)
      .BorderWidth(NavigationViewContentGridBorderThickness)
      .FlexGrow(1.0f)
      .FlexDirection(FlexDirection::Column)
      .Padding(NavigationViewPaneContentGridMargin),
  };
  return ret;
}

const ImmutableStyle& NavigationViewContentHeaderStyle() {
  using namespace StaticTheme::Common;
  static const ImmutableStyle ret {
    Style()
      /* You might notice this doesn't match the header padding/margin in the
       * WinUI3 gallery app's main window; this is because it does its' own
       * header inside the content area, instead of using the NavigationView's
       * 'header'.
       *
       * For a comparable example, see the 'window' example under the Title Bar
       * control samples instead.
       */
      .Margin(NavigationViewHeaderMargin)
      .Color(TextFillColorPrimary)
      .Font(SystemFont::Title)
      // overridden by NavigationView::SetHeaderText
      .Display(Display::None),

  };
  return ret;
}

const ImmutableStyle& NavigationViewContentInnerStyle() {
  static const ImmutableStyle ret {
    Style()
      .FlexGrow(1.0f)
      .FlexShrink(1.0f)
      .FlexDirection(FlexDirection::Column)
      .Margin(NavigationViewContentPresenterMargin)};
  return ret;
}

const ImmutableStyle& NavigationViewItemStyle() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::NavigationView;
  static const ImmutableStyle ret {
    Style()
      .Height(NavigationViewItemOnLeftMinHeight)
      .FlexDirection(FlexDirection::Row)
      .AlignItems(Align::Center)
      .AlignSelf(Align::Stretch)
      .Margin(NavigationViewItemButtonMargin)
      .BorderRadius(ControlCornerRadius)
      .BorderColor(NavigationViewItemBorderBrush)
      .BorderWidth(NavigationViewItemBorderThickness)
      .BackgroundColor(NavigationViewItemBackground)
      .Color(NavigationViewItemForeground)
      .Font(SystemFont::Body)
      .And(
        Hover,
        Style()
          .BackgroundColor(NavigationViewItemBackgroundPointerOver)
          .BorderColor(NavigationViewItemBorderBrushPointerOver)
          .Color(NavigationViewItemForegroundPointerOver))
      .And(
        Active,
        Style()
          .BackgroundColor(NavigationViewItemBackgroundPressed)
          .BorderColor(NavigationViewItemBorderBrushPressed)
          .Color(NavigationViewItemForegroundPressed))
      .And(
        Disabled,
        Style()
          .BackgroundColor(NavigationViewItemBackgroundDisabled)
          .BorderColor(NavigationViewItemBorderBrushDisabled)
          .Color(NavigationViewItemForegroundDisabled))
      .And(
        Checked,
        Style()
          .BackgroundColor(NavigationViewItemBackgroundSelected)
          .BorderColor(NavigationViewItemBorderBrushSelected)
          .Color(NavigationViewItemForegroundSelected)
          // Used if we ever supported top nav
          // .Font(SystemFont::BodyStrong)
          .And(
            Hover,
            Style()
              .BackgroundColor(NavigationViewItemBackgroundSelectedPointerOver)
              .BorderColor(NavigationViewItemBorderBrushSelectedPointerOver)
              .Color(NavigationViewItemForegroundSelectedPointerOver))
          .And(
            Active,
            Style()
              .BackgroundColor(NavigationViewItemBackgroundSelectedPressed)
              .BorderColor(NavigationViewItemBorderBrushSelectedPressed)
              .Color(NavigationViewItemForegroundSelectedPressed)))};
  return ret;
}
const ImmutableStyle& NavigationViewItemIconHolderStyle() {
  static const ImmutableStyle ret {
    Style()
      .Width(PaneToggleButtonWidth)
      .FlexDirection(FlexDirection::Row)
      .AlignContent(Align::Center)
      .AlignItems(Align::Center)
      .AlignSelf(Align::Center)
      .JustifyContent(Justify::Center)
      .TranslateX(-1),
  };
  return ret;
}

const ImmutableStyle& NavigationViewItemIconStyle() {
  static const ImmutableStyle ret {
    Style()
      .Font(SystemFont::ResolveGlyphFont(16))
      .TransformOriginX(0.5)
      .TransformOriginY(0.5),
  };
  return ret;
}

const ImmutableStyle& NavigationViewItemLabelStyle() {
  static const ImmutableStyle ret {
    Style()
      // Overridden by Pane when expanded
      .Display(Display::None),
  };
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::NavigationView
