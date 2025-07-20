// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxPopup.hpp"

#include <ComboBox.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "FredEmmott/GUI/Widgets/PopupWindow.hpp"
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "PopupWindow.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {
ComboBoxPopupResult BeginComboBoxPopup(bool* open, ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginComboBoxPopup(id);
  return *open;
}

ComboBoxPopupResult BeginComboBoxPopup(ID id) {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  using namespace immediate_detail;
  auto button = GetCurrentNode();
  if (!BeginBasicPopupWindow(id).Transparent()) {
    return false;
  }
  static const auto OuterStyles
    = Style()
        .BackgroundColor(ComboBoxDropDownBackground)
        .BorderColor(ComboBoxDropDownBorderBrush)
        .BorderRadius(OverlayCornerRadius)
        .BorderWidth(ComboBoxDropdownBorderThickness)
        .Padding(ComboBoxDropdownBorderPadding);
  static const auto InnerStyles
    = Style()
        .BorderRadius(OverlayCornerRadius)
        .Color(ComboBoxDropDownForeground)
        .FlexGrow(1.0)
        .Gap(0.0)
        .MarginBottom(-1.0)
        .MarginLeft(0.0)
        .MarginRight(0)
        .MarginTop(-0.5)
        .PaddingBottom(ComboBoxDropdownContentMarginBottom)
        .PaddingLeft(ComboBoxDropdownContentMarginLeft)
        .PaddingRight(ComboBoxDropdownContentMarginRight)
        .PaddingTop(ComboBoxDropdownContentMarginTop);

  const auto width = YGNodeLayoutGetWidth(button->GetLayoutNode()) + 8;

  BeginWidget<Widget>(ID {0});
  GetCurrentParentNode()->BuiltInStyles() = OuterStyles;
  BeginVStackPanel();
  auto& innerStyles = GetCurrentParentNode()->BuiltInStyles();
  innerStyles += InnerStyles;
  innerStyles.MinWidth() = width;
  return true;
}

void EndComboBoxPopup() {
  using namespace immediate_detail;
  EndStackPanel();
  EndWidget<Widget>();
  EndBasicPopupWindow();
}

}// namespace FredEmmott::GUI::Immediate