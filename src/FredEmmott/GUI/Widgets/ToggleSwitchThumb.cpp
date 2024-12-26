// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchThumb.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

ToggleSwitchThumb::ToggleSwitchThumb(std::size_t id) : Widget(id) {
}
void ToggleSwitchThumb::SetIsOn(bool value) noexcept {
  mIsOn = value;
}

WidgetStyles ToggleSwitchThumb::GetDefaultStyles() const {
  using namespace StaticTheme;
  constexpr auto KeySpline = ControlFastOutSlowInKeySpline<SkScalar>;
  constexpr CubicBezierStyleTransition FasterAnimation {
    ControlFasterAnimationDuration, KeySpline};
  constexpr CubicBezierStyleTransition NormalAnimation {
    ControlNormalAnimationDuration, KeySpline};

  constexpr auto parentWidth = Spacing * 10;
  constexpr auto margin = Spacing;
  constexpr auto height = Spacing * 3;

  constexpr auto hoverHeight = Spacing * 3.5;
  constexpr auto hoverMargin = Spacing * 0.75f;
  constexpr auto activeWidth = hoverHeight + (Spacing / 2);

  static const WidgetStyles baseStyles {
    .mBase = {
      .mBorderRadius = { Spacing * 2, FasterAnimation },
      .mHeight = { height, FasterAnimation },
      .mLeft = {0, NormalAnimation},
      .mMargin = { margin, FasterAnimation },
      .mMarginLeft = { margin, FasterAnimation },
      .mWidth = { height, FasterAnimation },
    },
    .mHover {
      .mBorderRadius = hoverHeight / 2,
      .mHeight = hoverHeight,
      .mMargin = hoverMargin,
      .mMarginLeft = hoverMargin, // TODO: inherit from margin
      .mWidth = hoverHeight,
    },
    .mActive = {
      .mWidth = activeWidth
    },
  };
  static const WidgetStyles offStyles {
    .mBase = {
      .mBackgroundColor = TextFillColorSecondaryBrush,
    },
  };
  // Positioning with `mMarginLeft` instead of align-self so that we
  // can animate it
  static const WidgetStyles onStyles {
    .mBase
    = {
      .mBackgroundColor = TextOnAccentFillColorPrimaryBrush,
      .mBorderColor = CircleElevationBorderBrush,
      .mLeft = parentWidth - (height + (2 * margin)),
    },
    .mActive = {
      .mMarginLeft = hoverMargin + hoverHeight - activeWidth,
    },
  };
  return baseStyles + (IsOn() ? onStyles : offStyles);
}

}// namespace FredEmmott::GUI::Widgets
