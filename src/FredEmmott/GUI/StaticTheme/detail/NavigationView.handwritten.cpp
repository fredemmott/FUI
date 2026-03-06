// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/StaticTheme/NavigationView.hpp>

namespace FredEmmott::GUI::StaticTheme::NavigationView {

const ImmutableStyle& NavigationViewStyle() {
  using namespace StaticTheme::NavigationView;
  static const ImmutableStyle ret {
    Style()
      .FlexDirection(YGFlexDirectionRow)
      .AlignItems(YGAlignStretch)
      .AlignSelf(YGAlignStretch)
      .FlexGrow(1),
  };
  return ret;
}

const ImmutableStyle& NavigationViewPaneStyle() {
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(NavigationViewDefaultPaneBackground)
      .AlignItems(YGAlignCenter)
      .FlexDirection(YGFlexDirectionColumn)
      .Width(NavigationViewCompactPaneLength)
      .And(
        NavigationViewPaneExpandedStyleClass,
        Style()
          .AlignItems(YGAlignFlexStart)
          .BackgroundColor(NavigationViewExpandedPaneBackground)
          .Width(320)// Literal in C++, not in xaml or named constant
        )};
  return ret;
}
const ImmutableStyle& NavigationViewContentOuterStyle() {
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(NavigationViewContentBackground)
      .BorderColor(NavigationViewContentGridBorderBrush)
      .BorderRadius(NavigationViewContentGridCornerRadius)
      .BorderLeftWidth(NavigationViewContentGridBorderThicknessLeft)
      .BorderTopWidth(NavigationViewContentGridBorderThicknessTop)
      .BorderRightWidth(NavigationViewContentGridBorderThicknessRight)
      .BorderBottomWidth(NavigationViewContentGridBorderThicknessBottom)
      .FlexGrow(1.0f)
      .FlexDirection(YGFlexDirectionColumn)
      .PaddingLeft(NavigationViewPaneContentGridMarginLeft)
      .PaddingTop(NavigationViewPaneContentGridMarginTop)
      .PaddingRight(NavigationViewPaneContentGridMarginRight)
      .PaddingBottom(NavigationViewPaneContentGridMarginBottom),
  };
  return ret;
}

const ImmutableStyle& NavigationViewContentHeaderStyle() {
  using namespace StaticTheme::Common;
  static const ImmutableStyle ret {
    Style()
      .MarginLeft(NavigationViewHeaderMarginLeft)
      .MarginTop(NavigationViewHeaderMarginTop)
      .MarginRight(NavigationViewHeaderMarginRight)
      .MarginBottom(NavigationViewHeaderMarginBottom)
      .Color(TextFillColorPrimary)
      .Font(SystemFont::Title)
      // overridden by NavigationView::SetHeaderText
      .Display(YGDisplayNone),

  };
  return ret;
}

const ImmutableStyle& NavigationViewContentInnerStyle() {
  static const ImmutableStyle ret {
    Style()
      .FlexGrow(1.0f)
      .FlexShrink(1.0f)
      .FlexDirection(YGFlexDirectionColumn)
      .MarginLeft(NavigationViewContentPresenterMarginLeft)
      .MarginTop(NavigationViewContentPresenterMarginTop)
      .MarginRight(NavigationViewContentPresenterMarginRight)
      .MarginBottom(NavigationViewContentPresenterMarginBottom)};
  return ret;
}

const ImmutableStyle& NavigationViewItemStyle() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::NavigationView;
  static const ImmutableStyle ret {
    Style()
      .Height(NavigationViewItemOnLeftMinHeight)//
      .FlexDirection(YGFlexDirectionRow)
      .AlignItems(YGAlignCenter)
      .MarginLeft(NavigationViewItemMarginLeft)
      .MarginRight(NavigationViewItemMarginRight)
      .BorderRadius(ControlCornerRadius)//
      .And(
        Hover, Style().BackgroundColor(NavigationViewItemBackgroundPointerOver))
      .And(Active, Style().BackgroundColor(NavigationViewItemBackgroundPressed))
      .And(
        Checked,
        Style().BackgroundColor(NavigationViewItemBackgroundSelected))};
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::NavigationView
