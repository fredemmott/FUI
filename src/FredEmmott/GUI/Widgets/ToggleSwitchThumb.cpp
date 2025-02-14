// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchThumb.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ToggleSwitch.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

namespace {
const auto ToggleSwitchThumbStyleClass = StyleClass::Make("ToggleSwitchThumb");
}

ToggleSwitchThumb::ToggleSwitchThumb(std::size_t id)
  : Widget(id, {ToggleSwitchThumbStyleClass}) {}

void ToggleSwitchThumb::SetIsOn(bool value) noexcept {
  mIsOn = value;
}

Style ToggleSwitchThumb::GetBuiltInStyles_DEPRECATED() const {
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

  using namespace PseudoClasses;
  static const Style baseStyles {
    .mBackgroundColor = { std::nullopt, ColorAnimation},
    .mBorderColor = { std::nullopt, ColorAnimation},
    .mBorderRadius = { height / 2, FasterAnimation },
    .mHeight = { height, FasterAnimation },
    .mLeft = {0, NormalAnimation},
    .mMargin = { margin, FasterAnimation },
    .mPosition = YGPositionTypeAbsolute,
    .mWidth = { height, FasterAnimation },
    .mAnd = {
      { Hover, Style {
        .mBorderRadius = hoverHeight / 2,
        .mHeight = hoverHeight,
        .mMargin = hoverMargin,
        .mWidth = hoverHeight,
       }},
      { Active, Style {
        .mWidth = activeWidth,
      }},
    },
  };
  static const Style offStyles {
    .mBackgroundColor = ToggleSwitchKnobFillOff,
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOffDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOffPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOffPointerOver,
      }},
    },
  };
  const float onLeft = parentWidth;
  static const Style onStyles {
    .mBackgroundColor = ToggleSwitchKnobFillOn,
    .mBorderColor = ToggleSwitchStrokeOn,
    .mLeft = onLeft - (height + 4 + margin),
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOnDisabled,
        .mBorderColor = ToggleSwitchStrokeOnDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOnPointerOver,
        .mBorderColor = ToggleSwitchStrokeOnPointerOver,
        .mMarginLeft = margin - (hoverHeight - height),
      }},
      { Active, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOnPressed,
        .mBorderColor = ToggleSwitchStrokeOnPressed,
        .mMarginLeft = margin - (activeWidth - height),
      }},
    },
  };
  return baseStyles + (IsOn() ? onStyles : offStyles);
}

}// namespace FredEmmott::GUI::Widgets
