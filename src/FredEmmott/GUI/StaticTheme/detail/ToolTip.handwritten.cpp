// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "ToolTip.handwritten.hpp"

#include <FredEmmott/GUI/StaticTheme/ToolTip.hpp>

namespace FredEmmott::GUI::StaticTheme::ToolTip {

const ImmutableStyle& DefaultToolTipStyle() {
  using namespace StaticTheme::Common;
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(ToolTipBackground)
      .BorderColor(ToolTipBorderBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(ToolTipBorderThemeThickness)
      .FlexDirection(FlexDirection::Column)
      .Font(
        SystemFont::Resolve(SystemFont::Body)
          .WithSize(ToolTipContentThemeFontSize))
      .Color(ToolTipForegroundBrush)
      .MaxWidth(ToolTipMaxWidth)
      .Padding(ToolTipBorderPadding),
  };
  return ret;
}
}// namespace FredEmmott::GUI::StaticTheme::ToolTip
