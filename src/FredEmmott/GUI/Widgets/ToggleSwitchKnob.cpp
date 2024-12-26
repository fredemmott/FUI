// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchKnob.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "ToggleSwitchThumb.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme;
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
  using namespace StaticTheme;
  constexpr CubicBezierStyleTransition FillAnimation {
    ControlFasterAnimationDuration, ControlFastOutSlowInKeySpline};
  static const WidgetStyles baseStyles {
    .mBase = {
      .mBackgroundColor = { std::nullopt, FillAnimation },
      .mBorderRadius = Spacing * 2.5f,
      .mBorderWidth = 1,
      .mFlexDirection = YGFlexDirectionColumn,
      .mHeight = Spacing * 5,
      .mWidth = Spacing * 10,
    },
  };
  static const WidgetStyles offStyles {
    .mBase = {
      .mBackgroundColor = ControlAltFillColorSecondaryBrush,
      .mBorderColor = ControlStrongStrokeColorDefaultBrush,
    },
    .mHover = {
      .mBackgroundColor = ControlAltFillColorTertiaryBrush,
    },
    .mActive = {
      .mBackgroundColor = ControlAltFillColorQuarternaryBrush,
    },
  };
  static const WidgetStyles onStyles {
    .mBase = {
      .mBackgroundColor = AccentFillColorDefaultBrush,
      .mBorderColor = AccentFillColorDefaultBrush,
    },
    .mHover = {
      .mBackgroundColor = AccentFillColorSecondaryBrush,
      .mBorderColor = AccentFillColorSecondaryBrush,
    },
    .mActive = {
      .mBackgroundColor = AccentFillColorTertiaryBrush,
      .mBorderColor = AccentFillColorTertiaryBrush,
    },
  };
  return baseStyles + (this->IsOn() ? onStyles : offStyles);
}

WidgetList ToggleSwitchKnob::GetDirectChildren() const noexcept {
  return {mThumb.get()};
}

}// namespace FredEmmott::GUI::Widgets
