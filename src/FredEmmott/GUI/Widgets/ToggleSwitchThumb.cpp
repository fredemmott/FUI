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

namespace Styles {
using namespace StaticTheme::ToggleSwitch;
using namespace PseudoClasses;

constexpr auto FasterAnimation = CubicBezierStyleTransition(
  ControlFasterAnimationDuration,
  ControlFastOutSlowInKeySpline);
constexpr auto NormalAnimation = CubicBezierStyleTransition(
  ControlNormalAnimationDuration,
  ControlFastOutSlowInKeySpline);

constexpr auto ColorAnimation
  = LinearStyleTransition(ControlFasterAnimationDuration);

constexpr auto NormalHeight = 12;
constexpr auto NormalWidth = NormalHeight;
constexpr auto HoverHeight = 14;
constexpr auto Margin = 3;
constexpr auto HoverMarginDiff = (HoverHeight - NormalHeight) / 2;
constexpr auto ActiveWidth = 17;
constexpr auto ParentWidth = 40;

const Style& OuterOff() {
  static const Style lazyInit {
    .mHeight = NormalHeight,
    .mLeft = {Margin, NormalAnimation},
    .mPosition = YGPositionTypeAbsolute,
    .mTop = Margin,
    .mWidth = NormalWidth,
  };
  return lazyInit;
}
const Style& OuterOn() {
  static const Style lazyInit = OuterOff()
    + Style {
      .mLeft = ParentWidth - ((2 * Margin) + NormalWidth),
    };
  return lazyInit;
}

const Style& InnerBase() {
  static const Style lazyInit {
    .mAlignSelf = YGAlignStretch,
    .mBackgroundColor = {std::nullopt, ColorAnimation},
    .mBorderColor = {std::nullopt, ColorAnimation},
    .mBorderRadius = {NormalHeight / 2, FasterAnimation},
    .mFlexGrow = 1,
    .mMargin = {std::nullopt, FasterAnimation},
    .mAnd = {
      { Hover, Style {
        .mBorderRadius = HoverHeight / 2,
        .mMarginBottom = -HoverMarginDiff,
        .mMarginLeft = -HoverMarginDiff,
        .mMarginRight = -HoverMarginDiff,
        .mMarginTop = -HoverMarginDiff,
      }}
    },
  };
  return lazyInit;
}

const Style& InnerOn() {
  static const Style lazyInit = InnerBase() + Style {
    .mBackgroundColor = ToggleSwitchKnobFillOn,
    .mBorderColor = ToggleSwitchStrokeOn,
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOnDisabled,
        .mBorderColor = ToggleSwitchStrokeOnDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOnPointerOver,
        .mBorderColor = ToggleSwitchStrokeOnPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOnPressed,
        .mBorderColor = ToggleSwitchStrokeOnPressed,
        .mMarginLeft = NormalWidth - ActiveWidth, // negative
        .mMarginRight = (2 * -HoverMarginDiff),
      }},
    },
  };
  return lazyInit;
}
const Style& InnerOff() {
  static const Style lazyInit = InnerBase() + Style {
    .mBackgroundColor = ToggleSwitchKnobFillOff,
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOffDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOffPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ToggleSwitchKnobFillOffPressed,
        .mMarginLeft = 0,
        .mMarginRight = NormalWidth - ActiveWidth, // negative
      }},
    },
  };
  return lazyInit;
}
}// namespace Styles
}// namespace

ToggleSwitchThumb::ToggleSwitchThumb(std::size_t id)
  : Widget(id, {ToggleSwitchThumbStyleClass}) {
  this->SetChildren({mInner = new Widget(0)});

  this->UpdateStyles();
}

void ToggleSwitchThumb::SetIsOn(const bool value) noexcept {
  if (mIsOn == value) {
    return;
  }
  mIsOn = value;
  this->UpdateStyles();
}

void ToggleSwitchThumb::UpdateStyles() {
  this->SetBuiltInStyles(mIsOn ? Styles::OuterOn() : Styles::OuterOff());
  mInner->SetBuiltInStyles(mIsOn ? Styles::InnerOn() : Styles::InnerOff());
}

}// namespace FredEmmott::GUI::Widgets