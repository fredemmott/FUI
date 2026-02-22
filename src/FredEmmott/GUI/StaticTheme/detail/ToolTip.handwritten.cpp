// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToolTip.handwritten.hpp"

#include <FredEmmott/GUI/StaticTheme/ToolTip.hpp>

namespace FredEmmott::GUI::StaticTheme::ToolTip {

const ImmutableStyle& DefaultToolTipStyle() {
  using namespace StaticTheme::Common;
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(ToolTipBackgroundBrush)
      .BorderColor(ToolTipBorderBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(ToolTipBorderThemeThickness)
      .FlexDirection(YGFlexDirectionColumn)
      .Font(
        SystemFont::Resolve(SystemFont::Body)
          .WithSize(ToolTipContentThemeFontSize))
      .Color(ToolTipForegroundBrush)
      .MaxWidth(ToolTipMaxWidth)
      .PaddingLeft(ToolTipBorderPaddingLeft)
      .PaddingTop(ToolTipBorderPaddingTop)
      .PaddingBottom(ToolTipBorderPaddingBottom)
      .PaddingRight(ToolTipBorderPaddingRight),
  };
  return ret;
}
}// namespace FredEmmott::GUI::StaticTheme::ToolTip
