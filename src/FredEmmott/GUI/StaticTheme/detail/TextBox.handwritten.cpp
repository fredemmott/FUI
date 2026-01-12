// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBox.handwritten.hpp"

#include <FredEmmott/GUI/PseudoClasses.hpp>
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
      .PaddingLeft(12)
      .PaddingRight(12)
      .PaddingTop(5)
      .PaddingBottom(6)
      .And(
        Disabled,
        Style()
          .BackgroundColor(TextControlBackgroundDisabled)
          .BorderColor(TextControlBorderBrushDisabled)
          .Color(TextControlForegroundDisabled))
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
          .Color(TextControlForegroundFocused)),
  };
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::TextBox
