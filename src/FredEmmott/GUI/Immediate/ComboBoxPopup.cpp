// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxPopup.hpp"

#include <ComboBox.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "PopupWindow.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {
bool BeginComboBoxPopup(bool* open, ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginComboBoxPopup(id);
  return *open;
}

bool BeginComboBoxPopup(ID id) {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  using namespace immediate_detail;
  if (!BeginPopupWindow(id)) {
    return false;
  }
  BeginVStackPanel();
  GetCurrentParentNode()->SetAdditionalBuiltInStyles( {
    .mBase = {
      .mBackgroundColor = ComboBoxDropDownBackground,
      .mBorderColor = ComboBoxDropDownBorderBrush,
      .mBorderRadius = OverlayCornerRadius,
      .mBorderWidth = ComboBoxDropdownBorderThickness,
      .mColor = ComboBoxDropDownForeground,
      .mGap = 0,
      .mMinWidth = 80,
      .mPadding = ComboBoxDropdownBorderPadding,
    },
    .mHover = {
      .mBackgroundColor = ComboBoxDropDownBackgroundPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ComboBoxDropDownBackgroundPointerPressed,
    },
  });
  // TODO
  return true;
}

void EndComboBoxPopup() {
  EndStackPanel();
  EndPopupWindow();
}

}// namespace FredEmmott::GUI::Immediate