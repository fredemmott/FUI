// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/Widgets/ComboBoxItemButton.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {
using Widgets::ComboBoxItemButton;

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
      .Height(ComboBoxItemPillHeight)
      .ScaleY(0, PillHeightAnimation)
      .MarginLeft(0.5)
      .MarginRight(6)
      .MarginTop(2.5)
      .TranslateY(0, PillHeightAnimation)
      .Width(ComboBoxItemPillWidth)
      .And(
        Checked,
        Style().ScaleY(1.0f).And(
          Active,
          Style()
            .ScaleY(ComboBoxItemPillMinScale)
            .TranslateY(
              (ComboBoxItemPillHeight
               - (ComboBoxItemPillHeight * ComboBoxItemPillMinScale))
              / 2))),
  };
  return ret;
}

}// namespace

ComboBoxItemResult<&EndComboBoxItem, void>
BeginComboBoxItem(bool* clicked, bool initiallySelected, const ID id) {
  using namespace immediate_detail;
  const auto item = BeginWidget<ComboBoxItemButton>(id);
  if (clicked) {
    *clicked = item->ConsumeWasActivated();
  }

  const bool isSelected = initiallySelected || (clicked && *clicked);
  item->SetIsChecked(isSelected);

  ChildlessWidget<Widget>(
    ID {"pill"}, LiteralStyleClass {"ComboBox/pill"}, ComboBoxItemPillStyles());
  static const ImmutableStyle ContentStyles {
    Style().MinHeight(
      StaticTheme::ComboBox::ComboBoxItemPillHeight
      + ComboBoxItemPillStyles()->MarginTop().value_or(0)
      + ComboBoxItemPillStyles()->MarginBottom().value_or(0)),
  };
  BeginWidget<Widget>(
    ID {"content"}, LiteralStyleClass {"ComboBox/content"}, ContentStyles);

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

ComboBoxItemResult<nullptr, bool>
ComboBoxItem(bool initiallySelected, std::string_view label, ID id) {
  bool clicked = false;
  const auto item = BeginComboBoxItem(&clicked, initiallySelected, id);
  Label(label, ID {0});
  EndComboBoxItem();
  return {item, clicked};
}

}// namespace FredEmmott::GUI::Immediate