// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {

class ComboBoxItemButton : public Widgets::Button {
 public:
  using Button::Button;

  using Widget::IsChecked;
  using Widget::SetIsChecked;
};

const Style& ComboBoxItemStyles() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;

  static const auto BaseStyles
    = Style()
        .BackgroundColor(ComboBoxItemBackground)
        .BorderColor(ComboBoxItemBorderBrush)
        .BorderRadius(ComboBoxItemCornerRadius)
        .Color(ComboBoxItemForeground)
        .MarginBottom(2)
        .MarginLeft(5)
        .MarginRight(5)
        .MarginTop(2)
        .PaddingBottom(ComboBoxItemThemePaddingBottom)
        .PaddingRight(ComboBoxItemThemePaddingRight)
        .PaddingTop(ComboBoxItemThemePaddingTop)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundDisabled)
            .BorderColor(ComboBoxItemBorderBrushDisabled)
            .Color(ComboBoxItemForegroundDisabled))
        .And(
          Hover,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundPointerOver)
            .BorderColor(ComboBoxItemBorderBrushPointerOver)
            .Color(ComboBoxItemForegroundPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundPressed)
            .BorderColor(ComboBoxItemBorderBrushPressed)
            .Color(ComboBoxItemForegroundPressed));
  static const auto SelectedStyles
    = Style()
        .BackgroundColor(ComboBoxItemBackgroundSelected)
        .BorderColor(ComboBoxItemBorderBrushSelected)
        .Color(ComboBoxItemForegroundSelected)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundSelectedDisabled)
            .BorderColor(ComboBoxItemBorderBrushSelectedDisabled)
            .Color(ComboBoxItemForegroundSelectedDisabled))
        .And(
          Hover,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundSelectedPointerOver)
            .BorderColor(ComboBoxItemBorderBrushSelectedPointerOver)
            .Color(ComboBoxItemForegroundSelectedPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(ComboBoxItemBackgroundSelectedPressed)
            .BorderColor(ComboBoxItemBorderBrushSelectedPressed)
            .Color(ComboBoxItemForegroundSelectedPressed));

  static auto Combined = BaseStyles + Style().And(Checked, SelectedStyles);
  return Combined;
}

const Style& ComboBoxItemPillStyles() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;

  constexpr auto PillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  static const auto PillStyles
    = Style()
        .BackgroundColor(ComboBoxItemPillFillBrush)
        .BorderRadius(ComboBoxItemPillCornerRadius)
        .Height(0, PillHeightAnimation)
        .MarginLeft(0.5)
        .MarginRight(6)
        .MarginTop(2.5)
        .Top(0, PillHeightAnimation)
        .Width(ComboBoxItemPillWidth)
        .And(
          Checked,
          Style()
            .Height(ComboBoxItemPillHeight)
            .And(
              Active,
              Style()
                .Height(ComboBoxItemPillHeight * ComboBoxItemPillMinScale)
                .Top(
                  (ComboBoxItemPillHeight
                   - (ComboBoxItemPillHeight * ComboBoxItemPillMinScale))
                  / 2)));
  return PillStyles;
}

}// namespace

Result<&EndComboBoxItem>
BeginComboBoxItem(bool* clicked, bool initiallySelected, ID id) {
  using namespace immediate_detail;
  const auto item = BeginWidget<ComboBoxItemButton>(id);
  item->BuiltInStyles() = ComboBoxItemStyles();
  if (clicked) {
    *clicked = std::exchange(item->mClicked, false);
  }

  const bool isSelected = initiallySelected || (clicked && *clicked);
  item->SetIsChecked(isSelected);

  BeginHStackPanel();
  GetCurrentParentNode()->BuiltInStyles() += Style().Gap(0.f);
  const auto pill = BeginWidget<Widget>(ID {"pill"});
  pill->BuiltInStyles() = ComboBoxItemPillStyles();

  EndWidget<Widget>();
  static const auto ContentStyles = Style() = Style().MinHeight(
    StaticTheme::ComboBox::ComboBoxItemPillHeight
    + ComboBoxItemPillStyles().MarginTop().value_or(0)
    + ComboBoxItemPillStyles().MarginBottom().value_or(0));
  BeginWidget<Widget>(ID {"content"})->BuiltInStyles() = ContentStyles;

  if (isSelected) {
    FUI_ASSERT(tWindow);
    tWindow->OffsetPositionToDescendant(GetCurrentParentNode());
  }

  return {item};
}

void EndComboBoxItem() {
  using namespace immediate_detail;
  EndWidget<Widget>();// content
  EndStackPanel();
  EndWidget<ComboBoxItemButton>();
}

Result<nullptr, bool>
ComboBoxItem(bool initiallySelected, std::string_view label, ID id) {
  bool clicked = false;
  const auto item = BeginComboBoxItem(&clicked, initiallySelected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return {item, clicked};
}

}// namespace FredEmmott::GUI::Immediate