// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.handwritten.hpp"

#include <FredEmmott/GUI/PseudoClasses.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>
#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::Button {
using namespace StaticTheme::Common;
using namespace PseudoClasses;

const style_detail::lazy_init_style DefaultButtonStyle {[] {
  return Style()
    .BackgroundColor(ButtonBackground)
    .BorderColor(ButtonBorderBrush)
    .BorderRadius(ControlCornerRadius)
    .BorderWidth(ButtonBorderThemeThickness)
    .Color(ButtonForeground)
    .Font(WidgetFont::ControlContent)
    .PaddingBottom(ButtonPaddingBottom)
    .PaddingLeft(ButtonPaddingLeft)
    .PaddingRight(ButtonPaddingRight)
    .PaddingTop(ButtonPaddingTop)
    .TextAlign(TextAlign::Center)
    .And(
      Disabled,
      Style()
        .BackgroundColor(ButtonBackgroundDisabled)
        .BorderColor(ButtonBorderBrushDisabled)
        .Color(ButtonForegroundDisabled))
    .And(
      Hover,
      Style()
        .BackgroundColor(ButtonBackgroundPointerOver)
        .BorderColor(ButtonBorderBrushPointerOver)
        .Color(ButtonForegroundPointerOver))
    .And(
      Active,
      Style()
        .BackgroundColor(ButtonBackgroundPressed)
        .BorderColor(ButtonBorderBrushPressed)
        .Color(ButtonForegroundPressed));
}};

const style_detail::lazy_init_style AccentButtonStyle {[] {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::Button;
  using namespace PseudoClasses;
  return DefaultButtonStyle
    + Style()
        .BackgroundColor(AccentButtonBackground)
        .BorderColor(AccentButtonBorderBrush)
        .Color(AccentButtonForeground)
        .And(
          Disabled,
          Style()
            .BackgroundColor(AccentButtonBackgroundDisabled)
            .BorderColor(AccentButtonBorderBrushDisabled)
            .Color(AccentButtonForegroundDisabled))
        .And(
          Hover,
          Style()
            .BackgroundColor(AccentButtonBackgroundPointerOver)
            .BorderColor(AccentButtonBorderBrushPointerOver)
            .Color(AccentButtonForegroundPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(AccentButtonBackgroundPressed)
            .BorderColor(AccentButtonBorderBrushPressed)
            .Color(AccentButtonForegroundPressed));
}};
}// namespace FredEmmott::GUI::StaticTheme::Button
