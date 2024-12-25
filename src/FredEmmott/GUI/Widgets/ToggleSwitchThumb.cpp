// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchThumb.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

ToggleSwitchThumb::ToggleSwitchThumb(std::size_t id) : Widget(id) {
}

WidgetStyles ToggleSwitchThumb::GetDefaultStyles() const {
  using namespace StaticTheme;
  const auto hoverHeight = Spacing * 3.5;
  static const WidgetStyles baseStyles {
    .mBase = {
      .mBorderRadius = Spacing * 2,
      .mHeight = Spacing * 3,
      .mMargin = Spacing,
      .mWidth = Spacing * 3,
    },
    .mHover {
      .mBorderRadius = hoverHeight / 2,
      .mHeight = hoverHeight,
      .mMargin = Spacing * 0.75f,
      .mWidth = hoverHeight,
    },
    .mActive = {
      .mWidth = hoverHeight + Spacing,
    },
  };
  static const WidgetStyles offStyles {
    .mBase = {
      .mAlignSelf = YGAlignFlexStart,
      .mBackgroundColor = TextFillColorSecondaryBrush,
    },
  };
  static const WidgetStyles onStyles {
    .mBase = {
      .mAlignSelf = YGAlignFlexEnd,
      .mBackgroundColor = TextOnAccentFillColorPrimaryBrush,
      .mBorderColor = CircleElevationBorderBrush,
    },
  };
  return baseStyles + (mIsOn ? onStyles : offStyles);
}

}// namespace FredEmmott::GUI::Widgets
