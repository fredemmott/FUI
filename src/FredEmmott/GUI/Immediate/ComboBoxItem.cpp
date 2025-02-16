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

  using namespace PseudoClasses;
  Style buttonStyles {
    .mBackgroundColor = ComboBoxItemBackground,
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
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ComboBoxItemBackgroundDisabled,
        .mBorderColor = ComboBoxItemBorderBrushDisabled,
        .mColor = ComboBoxItemForegroundDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ComboBoxItemBackgroundPointerOver,
        .mBorderColor = ComboBoxItemBorderBrushPointerOver,
        .mColor = ComboBoxItemForegroundPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ComboBoxItemBackgroundPressed,
        .mBorderColor = ComboBoxItemBorderBrushPressed,
        .mColor = ComboBoxItemForegroundPressed,
      }},
    },
  };
  if (isSelected) {
    buttonStyles += Style {
      .mBackgroundColor = ComboBoxItemBackgroundSelected,
      .mBorderColor = ComboBoxItemBorderBrushSelected,
      .mColor = ComboBoxItemForegroundSelected,
      .mAnd = {
        { Disabled, Style {
          .mBackgroundColor = ComboBoxItemBackgroundSelectedDisabled,
          .mBorderColor = ComboBoxItemBorderBrushSelectedDisabled,
          .mColor = ComboBoxItemForegroundSelectedDisabled,
        }},
        { Hover, Style {
          .mBackgroundColor = ComboBoxItemBackgroundSelectedPointerOver,
          .mBorderColor = ComboBoxItemBorderBrushSelectedPointerOver,
          .mColor = ComboBoxItemForegroundSelectedPointerOver,
        }},
        { Active, Style {
          .mBackgroundColor = ComboBoxItemBackgroundSelectedPressed,
          .mBorderColor = ComboBoxItemBorderBrushSelectedPressed,
          .mColor = ComboBoxItemForegroundSelectedPressed,
        }},
      },
    };
  }

  GetCurrentParentNode()->SetBuiltInStyles({buttonStyles});
  BeginHStackPanel();
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({.mGap = 0.0});
  BeginWidget<Widget>(ID {"pill"});

  const auto pillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  const SkScalar height = isSelected ? ComboBoxItemPillHeight : 0;
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({
    .mBackgroundColor = ComboBoxItemPillFillBrush,
    .mBorderRadius = ComboBoxItemPillCornerRadius,
    .mHeight = { height, pillHeightAnimation },
    .mMarginLeft = 0.5,
    .mMarginRight = 6,
    .mMarginTop = 2.5,
    .mTop = { 0, pillHeightAnimation },
    .mWidth = ComboBoxItemPillWidth,
    .mAnd = {
      { Active, Style {
        .mHeight = height * ComboBoxItemPillMinScale,
        .mTop = (height - (height * ComboBoxItemPillMinScale)) / 2,
      }},
    },
  });
  EndWidget<Widget>();
  BeginWidget<Widget>(ID {"content"});
  GetCurrentParentNode()->SetBuiltInStyles({
    .mDisplay = YGDisplayContents,
  });
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