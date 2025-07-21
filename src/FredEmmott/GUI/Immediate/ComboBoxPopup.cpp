// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxPopup.hpp"

#include <ComboBox.hpp>

#include "FredEmmott/GUI/Widgets/PopupWindow.hpp"
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "PopupWindow.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

namespace {
auto& OuterStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(ComboBoxDropDownBackground)
      .BorderColor(ComboBoxDropDownBorderBrush)
      .BorderRadius(OverlayCornerRadius)
      .BorderWidth(ComboBoxDropdownBorderThickness)
      .Padding(ComboBoxDropdownBorderPadding),
  };
  return ret;
}
auto& InnerStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  static const ImmutableStyle ret {
    Style()
      .BorderRadius(OverlayCornerRadius)
      .Color(ComboBoxDropDownForeground)
      .FlexDirection(YGFlexDirectionColumn)
      .FlexGrow(1.0)
      .Gap(0.0)
      .MarginBottom(-1.0)
      .MarginLeft(0.0)
      .MarginRight(0)
      .MarginTop(-0.5)
      .PaddingBottom(ComboBoxDropdownContentMarginBottom)
      .PaddingLeft(ComboBoxDropdownContentMarginLeft)
      .PaddingRight(ComboBoxDropdownContentMarginRight)
      .PaddingTop(ComboBoxDropdownContentMarginTop),
  };
  return ret;
}
}// namespace

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
  auto button = GetCurrentNode();
  if (!BeginBasicPopupWindow(id).Transparent()) {
    return false;
  }

  const auto width = YGNodeLayoutGetWidth(button->GetLayoutNode()) + 8;

  BeginWidget<Widget>(ID {0}, OuterStyles());
  BeginWidget<Widget>(ID {0}, InnerStyles())
    ->ReplaceExplicitStyles(Style().MinWidth(width));
  return true;
}

void EndComboBoxPopup() {
  EndWidget();
  EndWidget();
  EndBasicPopupWindow();
}

}// namespace FredEmmott::GUI::Immediate