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
      // TODO: set this when *not* bleeding into title bar
      // .BackgroundColor(NavigationViewDefaultPaneBackground)
      .AlignItems(YGAlignCenter)
      .FlexDirection(YGFlexDirectionColumn)
      .Width(NavigationViewCompactPaneLength)
      .And(
        NavigationViewPaneExpandedStyleClass,
        Style()
          .AlignItems(YGAlignFlexStart)
          .BackgroundColor(NavigationViewExpandedPaneBackground)
          .Width(320)// Literal in C++, not in xaml or named constant
        )
      .Descendants(
        NavigationViewItemLabelStyleClass, Style().Display(YGDisplayFlex))};
  return ret;
}

const ImmutableStyle& NavigationViewItemsRootStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignSelf(YGAlignStretch)
      .FlexDirection(YGFlexDirectionColumn)
      .FlexGrow(1)
      .AlignItems(YGAlignFlexStart),
  };
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
      /* You might notice this doesn't match the header padding/margin in the
       * WinUI3 gallery app's main window; this is because it does its' own
       * header inside the content area, instead of using the NavigationView's
       * 'header'.
       *
       * For a comparable example, see the 'window' example under the Title Bar
       * control samples instead.
       */
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
      .Height(NavigationViewItemOnLeftMinHeight)
      .FlexDirection(YGFlexDirectionRow)
      .AlignItems(YGAlignCenter)
      .AlignSelf(YGAlignStretch)
      .MarginLeft(NavigationViewItemButtonMarginLeft)
      .MarginTop(NavigationViewItemButtonMarginTop)
      .MarginRight(NavigationViewItemButtonMarginRight)
      .MarginBottom(NavigationViewItemButtonMarginBottom)
      .PaddingLeft(NavigationViewItemInnerHeaderMarginLeft)
      .PaddingTop(NavigationViewItemInnerHeaderMarginTop)
      .PaddingRight(NavigationViewItemInnerHeaderMarginRight)
      .PaddingBottom(NavigationViewItemInnerHeaderMarginBottom)
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

const ImmutableStyle& NavigationViewItemIconStyle() {
  static const ImmutableStyle ret {
    Style().Font(SystemFont::ResolveGlyphFont(16))};
  return ret;
}
const ImmutableStyle& NavigationViewItemLabelStyle() {
  static const ImmutableStyle ret {
    // Overridden by Pane when expanded
    Style()
      .Display(YGDisplayNone)
      .MarginLeft(NavigationViewItemInnerHeaderMarginLeft)
      .MarginTop(NavigationViewItemInnerHeaderMarginTop)
      .MarginRight(NavigationViewItemInnerHeaderMarginRight)
      .MarginBottom(NavigationViewItemInnerHeaderMarginBottom),
  };
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::NavigationView
