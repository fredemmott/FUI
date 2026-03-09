// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ComboBoxItemButton.hpp"

#include "FredEmmott/GUI/StaticTheme/ComboBox.hpp"
#include "FredEmmott/GUI/StaticTheme/Common.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {

constexpr LiteralStyleClass ComboBoxItemButtonStyleClass("ComboBox/ItemButton");

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

}// namespace

ComboBoxItemButton::ComboBoxItemButton(const id_type id)
  : Button(id, ComboBoxItemStyles(), {ComboBoxItemButtonStyleClass}),
    ISelectionItem(this) {}

ComboBoxItemButton::~ComboBoxItemButton() {}

}// namespace FredEmmott::GUI::Widgets
