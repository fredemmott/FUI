// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchKnob.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ToggleSwitch.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "ToggleSwitchThumb.hpp"
#include "WidgetList.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

ToggleSwitchKnob::ToggleSwitchKnob(std::size_t id) : Widget(id) {
  this->ChangeDirectChildren(
    [this] { mThumb.reset(new ToggleSwitchThumb({})); });
}

bool ToggleSwitchKnob::IsOn() const noexcept {
  return mThumb->IsOn();
}

void ToggleSwitchKnob::SetIsOn(bool value) noexcept {
  mThumb->SetIsOn(value);
}

WidgetStyles ToggleSwitchKnob::GetDefaultStyles() const {
  namespace Common = StaticTheme::Common;
  using namespace StaticTheme::ToggleSwitch;
  constexpr auto ColorAnimation
    = LinearStyleTransition(Common::ControlFasterAnimationDuration);

  static const WidgetStyles baseStyles {
    .mBase = {
      .mBackgroundColor = { std::nullopt, ColorAnimation },
      .mBorderColor = { std::nullopt, ColorAnimation },
      .mBorderRadius = Spacing * 2.5f,
      .mBorderWidth = ToggleSwitchOuterBorderStrokeThickness,
      .mFlexBasis = 0,
      .mFlexDirection = YGFlexDirectionColumn,
      .mHeight = 20,
      .mMarginRight = ToggleSwitchPreContentMargin,
      .mWidth = Spacing * 10,
    },
  };
  static const WidgetStyles offStyles {
    .mBase = {
      .mBackgroundColor = ToggleSwitchFillOff,
      .mBorderColor = ToggleSwitchStrokeOff,
    },
    .mDisabled = {
      .mBackgroundColor = ToggleSwitchFillOffDisabled,
      .mBorderColor = ToggleSwitchStrokeOffDisabled,
    },
    .mHover = {
      .mBackgroundColor = ToggleSwitchFillOffPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ToggleSwitchFillOffPressed,
    },
  };
  static const WidgetStyles onStyles {
    .mBase = {
      .mBackgroundColor = ToggleSwitchFillOn,
      .mBorderColor = ToggleSwitchStrokeOn,
    },
    .mDisabled = {
      .mBackgroundColor = ToggleSwitchFillOnDisabled,
      .mBorderColor = ToggleSwitchStrokeOnDisabled,
    },
    .mHover = {
      .mBackgroundColor = ToggleSwitchFillOnPointerOver,
      .mBorderColor = ToggleSwitchStrokeOnPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ToggleSwitchFillOnPressed,
      .mBorderColor = ToggleSwitchStrokeOnPressed,
    },
  };
  return baseStyles + (this->IsOn() ? onStyles : offStyles);
}

WidgetList ToggleSwitchKnob::GetDirectChildren() const noexcept {
  return {mThumb.get()};
}

}// namespace FredEmmott::GUI::Widgets
