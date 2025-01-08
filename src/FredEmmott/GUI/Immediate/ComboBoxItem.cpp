// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "Button.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

void BeginComboBoxItem(bool* selected, ID id) {
  using namespace immediate_detail;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  BeginButton(selected, id);
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
}

void EndComboBoxItem() {
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