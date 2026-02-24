// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.handwritten.hpp"

#include <FredEmmott/GUI/PseudoClasses.hpp>
#include <FredEmmott/GUI/StaticTheme/RepeatButton.hpp>
#include <FredEmmott/GUI/StaticTheme/TextBox.hpp>
#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::TextBox {
using namespace StaticTheme::Common;
using namespace PseudoClasses;

const ImmutableStyle& DefaultTextBoxStyle() {
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(TextControlBackground)
      .BorderColor(TextControlBorderBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(1)
      .Color(TextControlForeground)
      .FlexDirection(YGFlexDirectionRow)
      .PaddingLeft(12)
      .PaddingRight(12)
      .PaddingTop(5)
      .PaddingBottom(6)
      .Cursor(Cursor::Text)
      .Descendants(
        TextBoxButtonInvisibleWhenInactiveStyleClass,
        Style()
          .Opacity(0)
          .PointerEvents(PointerEvents::None)
          .And(Disabled, Style().Opacity(0)))
      .And(
        Disabled,
        Style()
          .BackgroundColor(TextControlBackgroundDisabled)
          .BorderColor(TextControlBorderBrushDisabled)
          .Color(TextControlForegroundDisabled)
          .Cursor(Cursor::Default))
      .And(
        Hover,
        Style()
          .BackgroundColor(TextControlBackgroundPointerOver)
          .BorderColor(TextControlBorderBrushPointerOver)
          .Color(TextControlForegroundPointerOver))
      .And(
        Focus,
        Style()
          .BackgroundColor(TextControlBackgroundFocused)
          .BorderColor(TextControlBorderBrushFocused)
          .Color(TextControlForegroundFocused)
          .Descendants(
            TextBoxButtonInvisibleWhenInactiveStyleClass,
            Style().Opacity(1).PointerEvents(PointerEvents::Auto)))
      // TextBox has its own focus indication (typically a colored underline),
      // we don't want the standard white rounded rect
      .And(FocusVisible, Style().OutlineWidth(0)),
  };
  return ret;
}
const ImmutableStyle& DefaultTextBoxButtonStyle() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Generic;
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(TextControlButtonBackground)
      .BorderColor(TextControlButtonBorderBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(Generic::TextControlBorderThemeThickness)
      .Color(TextControlForeground)
      .Cursor(Cursor::Default)
      .PaddingLeft(HelperButtonThemePaddingLeft)
      .PaddingTop(HelperButtonThemePaddingTop)
      .PaddingBottom(HelperButtonThemePaddingBottom)
      .PaddingRight(HelperButtonThemePaddingRight)
      .And(
        Hover,
        Style()
          .BackgroundColor(TextControlButtonBackgroundPointerOver)
          .BorderColor(TextControlButtonBorderBrushPointerOver)
          .Color(TextControlButtonForegroundPointerOver))
      .And(
        Active,
        Style()
          .BackgroundColor(TextControlButtonBackgroundPressed)
          .BorderColor(TextControlButtonBorderBrushPressed)
          .Color(TextControlButtonForegroundPressed))
      .And(
        Disabled,
        Style()
          .BackgroundColor(RepeatButtonBackgroundDisabled)
          .BorderColor(RepeatButtonBorderBrushDisabled)
          .Color(RepeatButtonForegroundDisabled))
      .Descendants(
        {},
        Style().Font(
          SystemFont::ResolveGlyphFont(SystemFont::Body)
            .WithSize(TextBoxIconFontSize),
          !important))};
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::TextBox
