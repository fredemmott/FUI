// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxPopup.hpp"

#include <ComboBox.hpp>

#include "FredEmmott/GUI/Widgets/Focusable.hpp"
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
      .MinWidth(ComboBoxPopupThemeMinWidth)
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
      .MinWidth(ComboBoxPopupThemeMinWidth)
      .PaddingBottom(ComboBoxDropdownContentMarginBottom)
      .PaddingLeft(ComboBoxDropdownContentMarginLeft)
      .PaddingRight(ComboBoxDropdownContentMarginRight)
      .PaddingTop(ComboBoxDropdownContentMarginTop),
  };
  return ret;
}

class ComboBoxList final : public Widgets::Widget,
                           public Widgets::ISelectionContainer {
 public:
  explicit ComboBoxList(const std::size_t id)
    : Widget(id, LiteralStyleClass {"ComboBox/List"}, InnerStyles(), {}) {}
};
}// namespace

ComboBoxPopupResult BeginComboBoxPopup(bool* open, const ID id) {
  if (!(open && *open)) {
    return false;
  }
  *open = BeginComboBoxPopup(id);
  return *open;
}

ComboBoxPopupResult BeginComboBoxPopup(const ID id) {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  auto button = GetCurrentNode();
  if (!BeginBasicPopupWindow(id).Transparent()) {
    return false;
  }

  const auto width = YGNodeLayoutGetWidth(button->GetLayoutNode()) + 8;

  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"ComboBox/Popup"}, OuterStyles());
  BeginWidget<ComboBoxList>(ID {0})->SetMutableStyles(
    Style().MinWidth(std::max(width, ComboBoxPopupThemeMinWidth)));
  return true;
}

void EndComboBoxPopup() {
  EndWidget<ComboBoxList>();
  EndWidget();
  EndBasicPopupWindow();
}

}// namespace FredEmmott::GUI::Immediate