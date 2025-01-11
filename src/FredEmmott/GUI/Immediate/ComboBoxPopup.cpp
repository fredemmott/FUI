// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxPopup.hpp"

#include <ComboBox.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "FredEmmott/GUI/Widgets/PopupWindow.hpp"
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
  auto button = GetCurrentNode();
  if (!BeginPopupWindow(id)) {
    return false;
  }
  if (tWindow && !tWindow->GetNativeHandle()) {
    tWindow->SetSystemBackdropType(DWMSBT_NONE);
  }
  const auto width = YGNodeLayoutGetWidth(button->GetLayoutNode()) + 8;

  BeginWidget<Widget>(ID {0});
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({{
    .mBackgroundColor = ComboBoxDropDownBackground,
    .mBorderColor = ComboBoxDropDownBorderBrush,
    .mBorderRadius = OverlayCornerRadius,
    .mBorderWidth = ComboBoxDropdownBorderThickness,
    .mPadding = ComboBoxDropdownBorderPadding,
  }});
  BeginVStackPanel();
  GetCurrentParentNode()->SetAdditionalBuiltInStyles( {
    .mBase = {
      .mBorderRadius = OverlayCornerRadius,
      .mColor = ComboBoxDropDownForeground,
      .mFlexGrow = 1.0,
      .mGap = 0.0,
      .mMarginBottom = -1.0,
      .mMarginLeft = 0.0,
      .mMarginRight = 0,
      .mMarginTop = -0.5,
      .mPaddingBottom = ComboBoxDropdownContentMarginBottom,
      .mPaddingLeft = ComboBoxDropdownContentMarginLeft,
      .mPaddingRight = ComboBoxDropdownContentMarginRight,
      .mPaddingTop = ComboBoxDropdownContentMarginTop,
      .mWidth = width,
    },
  });
  return true;
}

void EndComboBoxPopup() {
  using namespace immediate_detail;
  EndStackPanel();
  EndWidget<Widget>();
  EndPopupWindow();
}

}// namespace FredEmmott::GUI::Immediate