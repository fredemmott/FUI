// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItem.hpp"

#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/assert.hpp>

#include "Button.hpp"
#include "Label.hpp"
#include "StackPanel.hpp"

namespace FredEmmott::GUI::Immediate {

Result<&EndComboBoxItem>
BeginComboBoxItem(bool* clicked, bool initiallySelected, ID id) {
  using namespace immediate_detail;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  const auto ret = BeginButton(clicked, id);
  const bool isSelected = initiallySelected || (clicked && *clicked);

  using namespace PseudoClasses;
  auto buttonStyles = Style()
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
  if (isSelected) {
    buttonStyles
      += Style()
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
  }

  GetCurrentParentNode()->SetBuiltInStyles({buttonStyles});
  BeginHStackPanel();
  GetCurrentParentNode()->SetAdditionalBuiltInStyles(Style().Gap(0.f));
  BeginWidget<Widget>(ID {"pill"});

  const auto pillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  const float height = isSelected ? ComboBoxItemPillHeight : 0;
  GetCurrentParentNode()->SetAdditionalBuiltInStyles(
    Style()
      .BackgroundColor(ComboBoxItemPillFillBrush)
      .BorderRadius(ComboBoxItemPillCornerRadius)
      .Height(height, pillHeightAnimation)
      .MarginLeft(0.5)
      .MarginRight(6)
      .MarginTop(2.5)
      .Top(0, pillHeightAnimation)
      .Width(ComboBoxItemPillWidth)
      .And(
        Active,
        Style()
          .Height(height * ComboBoxItemPillMinScale)
          .Top((height - (height * ComboBoxItemPillMinScale)) / 2)));
  EndWidget<Widget>();
  BeginWidget<Widget>(ID {"content"});

  if (isSelected) {
    FUI_ASSERT(tWindow);
    tWindow->OffsetPositionToDescendant(GetCurrentParentNode());
  }

  return {ret};
}

void EndComboBoxItem() {
  using namespace immediate_detail;
  EndWidget<Widget>();// content
  EndStackPanel();
  EndButton();
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