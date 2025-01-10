// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginComboBoxItem(bool* clicked, bool initiallySelected, ID id) {
  using namespace immediate_detail;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  BeginButton(clicked, id);
  const bool isSelected = initiallySelected || (clicked && *clicked);
  if (isSelected) {
    FUI_ASSERT(tWindow);
    tWindow->OffsetPositionToDescendant(GetCurrentParentNode());
  }

  WidgetStyles buttonStyles {
    .mBase = {
      .mBackgroundColor = isSelected ? ComboBoxItemBackgroundSelected : ComboBoxItemBackground,
      .mBorderColor = ComboBoxItemBorderBrush,
      .mBorderRadius = ComboBoxItemCornerRadius,
      .mColor = ComboBoxItemForeground,
      .mMarginBottom = 2,
      .mMarginLeft = 5,
      .mMarginRight = 5,
      .mMarginTop = 2,
      .mPaddingBottom = ComboBoxItemThemePaddingBottom,
      .mPaddingRight = ComboBoxItemThemePaddingRight,
      .mPaddingTop = ComboBoxItemThemePaddingTop,
    },
    .mDisabled = {
      .mBackgroundColor = isSelected ? ComboBoxItemBackgroundSelectedDisabled : ComboBoxItemBackgroundDisabled,
      .mBorderColor = ComboBoxItemBorderBrushDisabled,
      .mColor = ComboBoxItemForegroundDisabled,
    },
    .mHover = {
      .mBackgroundColor = isSelected ? ComboBoxItemBackgroundSelectedPointerOver : ComboBoxItemBackgroundPointerOver,
      .mBorderColor = ComboBoxItemBorderBrushPointerOver,
      .mColor = ComboBoxItemForegroundPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ComboBoxItemBackgroundPressed,
      .mBorderColor = ComboBoxItemBorderBrushPressed,
      .mColor = ComboBoxItemForegroundPressed,
    },
  };
  if (isSelected) {
    buttonStyles += WidgetStyles {
      .mBase = {
        .mBackgroundColor = ComboBoxItemBackgroundSelected,
        .mBorderColor = ComboBoxItemBorderBrushSelected,
        .mColor = ComboBoxItemForegroundSelected,
      },
      .mDisabled = {
        .mBackgroundColor = ComboBoxItemBackgroundSelectedDisabled,
        .mBorderColor = ComboBoxItemBorderBrushSelectedDisabled,
        .mColor = ComboBoxItemForegroundSelectedDisabled,
      },
      .mHover = {
        .mBackgroundColor = ComboBoxItemBackgroundSelectedPointerOver,
        .mBorderColor = ComboBoxItemBorderBrushSelectedPointerOver,
        .mColor = ComboBoxItemForegroundSelectedPointerOver,
      },
      .mActive = {
        .mBackgroundColor = ComboBoxItemBackgroundSelectedPressed,
        .mBorderColor = ComboBoxItemBorderBrushSelectedPressed,
        .mColor = ComboBoxItemForegroundSelectedPressed,
      },
    };
  }

  GetCurrentParentNode()->SetBuiltInStyles(buttonStyles);
  BeginHStackPanel();
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({{.mGap = 0.0}});
  BeginWidget<Widget>(ID {"pill"});

  const auto pillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  const SkScalar height = isSelected ? ComboBoxItemPillHeight : 0;
  GetCurrentParentNode()->SetAdditionalBuiltInStyles(WidgetStyles {
    .mBase = Style {
      .mBackgroundColor = ComboBoxItemPillFillBrush,
      .mBorderRadius = ComboBoxItemPillCornerRadius,
      .mHeight = { height, pillHeightAnimation },
      .mMarginLeft = 0.5,
      .mMarginRight = 6,
      .mMarginTop = 2.5,
      .mTop = { 0, pillHeightAnimation },
      .mWidth = ComboBoxItemPillWidth,
    },
    .mActive = Style {
      .mHeight = height * ComboBoxItemPillMinScale,
      .mTop = (height - (height * ComboBoxItemPillMinScale)) / 2,
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
  bool clicked = false;
  BeginComboBoxItem(&clicked, initiallySelected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return clicked;
}

}// namespace FredEmmott::GUI::Immediate