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
  return Style {
    .mBackgroundColor = ButtonBackground,
    .mBorderColor = ButtonBorderBrush,
    .mBorderRadius = ControlCornerRadius,
    .mBorderWidth = ButtonBorderThemeThickness,
    .mColor = ButtonForeground,
    .mFont = WidgetFont::ControlContent,
    .mPaddingBottom = ButtonPaddingBottom,
    .mPaddingLeft = ButtonPaddingLeft,
    .mPaddingRight = ButtonPaddingRight,
    .mPaddingTop = ButtonPaddingTop,
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ButtonBackgroundDisabled,
        .mBorderColor = ButtonBorderBrushDisabled,
        .mColor = ButtonForegroundDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ButtonBackgroundPointerOver,
        .mBorderColor = ButtonBorderBrushPointerOver,
        .mColor = ButtonForegroundPointerOver,
        }},
        { Active, Style {
          .mBackgroundColor = ButtonBackgroundPressed,
          .mBorderColor = ButtonBorderBrushPressed,
          .mColor = ButtonForegroundPressed,
        }},
      },
    };
}};

const style_detail::lazy_init_style AccentButtonStyle {[] {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::Button;
  using namespace PseudoClasses;
  return DefaultButtonStyle
    + Style {
      .mBackgroundColor = AccentButtonBackground,
      .mBorderColor = AccentButtonBorderBrush,
      .mColor = AccentButtonForeground,
      .mAnd = {
        { Disabled, Style {
          .mBackgroundColor = AccentButtonBackgroundDisabled,
          .mBorderColor = AccentButtonBorderBrushDisabled,
          .mColor = AccentButtonForegroundDisabled,
        }},
        { Hover, Style {
          .mBackgroundColor = AccentButtonBackgroundPointerOver,
          .mBorderColor = AccentButtonBorderBrushPointerOver,
          .mColor = AccentButtonForegroundPointerOver,
        }},
        { Active, Style {
          .mBackgroundColor = AccentButtonBackgroundPressed,
          .mBorderColor = AccentButtonBorderBrushPressed,
          .mColor = AccentButtonForegroundPressed,
        }},
      },
    };
}};
}// namespace FredEmmott::GUI::StaticTheme::Button
