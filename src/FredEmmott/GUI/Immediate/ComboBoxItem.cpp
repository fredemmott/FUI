// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginComboBoxItem(bool* selectedInOut, ID id) {
  using namespace immediate_detail;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  bool clicked = false;
  BeginButton(&clicked, id);
  const bool isSelected = clicked || (selectedInOut && *selectedInOut);
  if (selectedInOut) {
    *selectedInOut = isSelected;
  }

  GetCurrentParentNode()->SetBuiltInStyles({
    .mBase = {
      .mBackgroundColor = ComboBoxItemBackground,
      .mBorderColor = ComboBoxItemBorderBrush,
      .mBorderRadius = ComboBoxItemCornerRadius,
      .mColor = ComboBoxItemForeground,
      .mMarginBottom = 2,
      .mMarginLeft = 5,
      .mMarginRight = 5,
      .mMarginTop = 2,
      .mPaddingBottom = ComboBoxItemThemePaddingBottom,
      .mPaddingLeft = ComboBoxItemThemePaddingLeft,
      .mPaddingRight = ComboBoxItemThemePaddingRight,
      .mPaddingTop = ComboBoxItemThemePaddingTop,
    },
    .mDisabled = {
      .mBackgroundColor = ComboBoxItemBackgroundDisabled,
      .mBorderColor = ComboBoxItemBorderBrushDisabled,
      .mColor = ComboBoxItemForegroundDisabled,
    },
    .mHover = {
      .mBackgroundColor = ComboBoxItemBackgroundPointerOver,
      .mBorderColor = ComboBoxItemBorderBrushPointerOver,
      .mColor = ComboBoxItemForegroundPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ComboBoxItemBackgroundPressed,
      .mBorderColor = ComboBoxItemBorderBrushPressed,
      .mColor = ComboBoxItemForegroundPressed,
    },
  });
  BeginHStackPanel();
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({{.mGap = 0.0}});
  BeginWidget<Widget>(ID {"pill"});

  const auto pillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({
    .mBase = {
      .mBackgroundColor = isSelected ?  ComboBoxItemPillFillBrush : Brush { SK_ColorTRANSPARENT },
      .mBorderRadius = ComboBoxItemPillCornerRadius,
      .mHeight = { ComboBoxItemPillHeight, pillHeightAnimation },
      .mMarginLeft = 0.5,
      .mMarginRight = 6,
      .mMarginTop = 0.5,
      .mTop = { 0, pillHeightAnimation },
      .mWidth = ComboBoxItemPillWidth,
    },
    .mActive {
      .mHeight = ComboBoxItemPillHeight * ComboBoxItemPillMinScale,
      .mTop = (ComboBoxItemPillHeight - (ComboBoxItemPillHeight * ComboBoxItemPillMinScale)) / 2,
    },
  });
  EndWidget<Widget>();
  BeginWidget<Widget>(ID {"content"});
  GetCurrentParentNode()->SetBuiltInStyles({{
    .mDisplay = YGDisplayContents,
  }});
}

void EndComboBoxItem() {
  using namespace immediate_detail;
  EndWidget<Widget>();// content
  EndStackPanel();
  EndButton();
}

bool ComboBoxItem(bool initiallySelected, std::string_view label, ID id) {
  bool selected = initiallySelected;
  BeginComboBoxItem(&selected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return selected;
}

}// namespace FredEmmott::GUI::Immediate