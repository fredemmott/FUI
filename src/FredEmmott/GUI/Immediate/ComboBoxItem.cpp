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

class ComboBoxItemButton : public Widgets::Button,
                           public Widgets::ISelectionItem {
 public:
  using Button::Button;

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  bool IsSelected() const noexcept override {
    return IsChecked();
  }

  void Select() override {
    this->Invoke();
  }
};

Style MakeComboBoxItemStyles() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;

  const auto BaseStyles
    = Style()
        .BackgroundColor(ComboBoxItemBackground)
        .BorderColor(ComboBoxItemBorderBrush)
        .BorderRadius(ComboBoxItemCornerRadius)
        .Color(ComboBoxItemForeground)
        .FlexDirection(YGFlexDirectionRow)
        .Gap(0)
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
  const auto SelectedStyles
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

  return BaseStyles + Style().And(Checked, SelectedStyles);
}

auto& ComboBoxItemStyles() {
  static const ImmutableStyle ret {MakeComboBoxItemStyles()};
  return ret;
}

auto& ComboBoxItemPillStyles() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;

  constexpr auto PillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  static const ImmutableStyle ret {
    Style()
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
                / 2))),
  };
  return ret;
}

}// namespace

Result<&EndComboBoxItem>
BeginComboBoxItem(bool* clicked, bool initiallySelected, const ID id) {
  using namespace immediate_detail;
  const auto item = BeginWidget<ComboBoxItemButton>(
    id, ComboBoxItemStyles(), StyleClasses {});
  if (clicked) {
    *clicked = std::exchange(item->mClicked, false);
  }

  const bool isSelected = initiallySelected || (clicked && *clicked);
  item->SetIsChecked(isSelected);

  BeginWidget<Widget>(ID {"pill"}, ComboBoxItemPillStyles());
  EndWidget<Widget>();
  static const ImmutableStyle ContentStyles {
    Style().MinHeight(
      StaticTheme::ComboBox::ComboBoxItemPillHeight
      + ComboBoxItemPillStyles()->MarginTop().value_or(0)
      + ComboBoxItemPillStyles()->MarginBottom().value_or(0)),
  };
  BeginWidget<Widget>(ID {"content"}, ContentStyles);

  if (isSelected) {
    FUI_ASSERT(tWindow);
    tWindow->OffsetPositionToDescendant(GetCurrentParentNode());
  }

  return {item};
}

void EndComboBoxItem() {
  using namespace immediate_detail;
  EndWidget<Widget>();// content
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