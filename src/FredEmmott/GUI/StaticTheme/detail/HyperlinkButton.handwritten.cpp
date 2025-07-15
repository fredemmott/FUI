// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "HyperlinkButton.handwritten.hpp"

#include <FredEmmott/GUI/StaticTheme/HyperlinkButton.hpp>
#include <FredEmmott/GUI/StyleTransition.hpp>

using namespace FredEmmott::GUI::StaticTheme::Common;
using namespace FredEmmott::GUI::StaticTheme::Generic;
namespace FredEmmott::GUI::StaticTheme::HyperlinkButton {

const style_detail::lazy_init_style DefaultHyperlinkButtonStyle {[] {
  using namespace PseudoClasses;
  // Hardcoded in xaml
  constexpr auto BackgroundTransition
    = LinearStyleTransition(std::chrono::milliseconds(83));
  return Style()
    .BackgroundColor(HyperlinkButtonBackground, BackgroundTransition)
    .Color(HyperlinkButtonForeground)
    .BorderColor(HyperlinkButtonBorderBrush)
    .BorderWidth(HyperlinkButtonBorderThemeThickness)
    .PaddingLeft(HyperlinkButtonPaddingLeft)
    .PaddingRight(HyperlinkButtonPaddingRight)
    .PaddingTop(HyperlinkButtonPaddingTop)
    .PaddingBottom(HyperlinkButtonPaddingBottom)
    .BorderRadius(ControlCornerRadius)
    .Cursor(Cursor::Pointer)// hand
    .And(
      Hover,
      Style()
        .BackgroundColor(HyperlinkButtonBackgroundPointerOver)
        .BorderColor(HyperlinkButtonBorderBrushPointerOver)
        .Color(HyperlinkButtonForegroundPointerOver))
    .And(
      Active,
      Style()
        .BackgroundColor(HyperlinkButtonBackgroundPressed)
        .BorderColor(HyperlinkButtonBorderBrushPressed)
        .Color(HyperlinkButtonForegroundPressed))
    .And(
      Disabled,
      Style()
        .BackgroundColor(HyperlinkButtonBackgroundDisabled)
        .BorderColor(HyperlinkButtonBorderBrushDisabled)
        .Color(HyperlinkButtonForegroundDisabled));
}};
}// namespace FredEmmott::GUI::StaticTheme::HyperlinkButton