// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.handwritten.hpp"

#include <FredEmmott/GUI/PseudoClasses.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>
#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::Button {
using namespace StaticTheme::Common;
using namespace PseudoClasses;

Style MakeBaseButtonStyle() {
  return Style()
    .BorderRadius(ControlCornerRadius)
    .BorderWidth(ButtonBorderThemeThickness)
    .Font(WidgetFont::ControlContent)
    .PaddingBottom(ButtonPaddingBottom)
    .PaddingLeft(ButtonPaddingLeft)
    .PaddingRight(ButtonPaddingRight)
    .PaddingTop(ButtonPaddingTop)
    .TextAlign(TextAlign::Center);
}

const ImmutableStyle& DefaultButtonStyle() {
  static const ImmutableStyle ret {
    MakeBaseButtonStyle()
      + Style()
          .And(*AccentButtonStyleClass, AccentButtonStyle().Get())
          .And(
            !*AccentButtonStyleClass,
            Style()
              .BackgroundColor(ButtonBackground)
              .BorderColor(ButtonBorderBrush)
              .Color(ButtonForeground)
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
                  .Color(ButtonForegroundPressed))),
  };
  return ret;
};

const ImmutableStyle& AccentButtonStyle() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::Button;
  using namespace PseudoClasses;

  static const ImmutableStyle ret {
    Style()
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
          .Color(AccentButtonForegroundPressed)),
  };
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::Button
