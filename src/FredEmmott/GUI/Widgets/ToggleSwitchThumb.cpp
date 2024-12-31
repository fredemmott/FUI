// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchThumb.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ToggleSwitch.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

ToggleSwitchThumb::ToggleSwitchThumb(std::size_t id) : Widget(id) {
}

void ToggleSwitchThumb::SetIsOn(bool value) noexcept {
  mIsOn = value;
}

WidgetStyles ToggleSwitchThumb::GetDefaultStyles() const {
  using namespace StaticTheme::ToggleSwitch;

  constexpr auto KeySpline = ControlFastOutSlowInKeySpline;
  constexpr auto FasterAnimation
    = CubicBezierStyleTransition(ControlFasterAnimationDuration, KeySpline);
  constexpr auto NormalAnimation
    = CubicBezierStyleTransition(ControlNormalAnimationDuration, KeySpline);

  constexpr auto ColorAnimation
    = LinearStyleTransition(ControlFasterAnimationDuration);

  constexpr auto parentWidth = Spacing * 10;
  constexpr auto height = 12;
  constexpr auto margin = 3;

  constexpr auto hoverHeight = 14;
  constexpr auto hoverMargin = margin - ((hoverHeight - height) / 2);
  constexpr auto activeWidth = hoverHeight + (Spacing / 2);

  static const WidgetStyles baseStyles {
    .mBase = {
      .mBackgroundColor = { std::nullopt, ColorAnimation},
      .mBorderColor = { std::nullopt, ColorAnimation},
      .mBorderRadius = { height / 2, FasterAnimation },
      .mHeight = { height, FasterAnimation },
      .mLeft = {0, NormalAnimation},
      .mMargin = { margin, FasterAnimation },
      .mPosition = YGPositionTypeAbsolute,
      .mWidth = { height, FasterAnimation },
    },
    .mHover {
      .mBorderRadius = hoverHeight / 2,
      .mHeight = hoverHeight,
      .mMargin = hoverMargin,
      .mWidth = hoverHeight,
    },
    .mActive = {
      .mWidth = activeWidth
    },
  };
  static const WidgetStyles offStyles {
    .mBase = {
      .mBackgroundColor = ToggleSwitchKnobFillOff,
    },
    .mDisabled = {
      .mBackgroundColor = ToggleSwitchKnobFillOffDisabled,
    },
    .mHover = {
      .mBackgroundColor = ToggleSwitchKnobFillOffPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ToggleSwitchKnobFillOffPointerOver,
    },
  };
  const float onLeft = parentWidth;
  static const WidgetStyles onStyles {
    .mBase = {
      .mBackgroundColor = ToggleSwitchKnobFillOn,
      .mBorderColor = ToggleSwitchStrokeOn,
      .mLeft = onLeft - (height + 4 + margin),
    },
    .mDisabled = {
      .mBackgroundColor = ToggleSwitchKnobFillOnDisabled,
      .mBorderColor = ToggleSwitchStrokeOnDisabled,
    },
    .mHover = {
      .mBackgroundColor = ToggleSwitchKnobFillOnPointerOver,
      .mBorderColor = ToggleSwitchStrokeOnPointerOver,
      .mMarginLeft = margin - (hoverHeight - height),
    },
    .mActive = {
      .mBackgroundColor = ToggleSwitchKnobFillOnPressed,
      .mBorderColor = ToggleSwitchStrokeOnPressed,
      .mMarginLeft = margin - (activeWidth - height),
    },
  };
  return baseStyles + (IsOn() ? onStyles : offStyles);
}

}// namespace FredEmmott::GUI::Widgets
