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

namespace {
const auto ToggleSwitchKnobStyleClass = StyleClass::Make("ToggleSwitchKnob");
}

using namespace widget_detail;

ToggleSwitchKnob::ToggleSwitchKnob(std::size_t id)
  : Widget(id, {ToggleSwitchKnobStyleClass}) {
  this->ChangeDirectChildren(
    [this] { mThumb.reset(new ToggleSwitchThumb({})); });
}

bool ToggleSwitchKnob::IsOn() const noexcept {
  return mThumb->IsOn();
}

void ToggleSwitchKnob::SetIsOn(bool value) noexcept {
  mThumb->SetIsOn(value);
}

Style ToggleSwitchKnob::GetBuiltInStyles() const {
  namespace Common = StaticTheme::Common;
  using namespace StaticTheme::ToggleSwitch;
  constexpr auto ColorAnimation
    = LinearStyleTransition(Common::ControlFasterAnimationDuration);

  using namespace PseudoClasses;
  static const Style baseStyles {
    .mBackgroundColor = {std::nullopt, ColorAnimation},
    .mBorderColor = {std::nullopt, ColorAnimation},
    .mBorderRadius = Spacing * 2.5f,
    .mBorderWidth = ToggleSwitchOuterBorderStrokeThickness,
    .mFlexBasis = Spacing * 10,
    .mFlexDirection = YGFlexDirectionColumn,
    .mHeight = 20,
    .mMarginRight = ToggleSwitchPreContentMargin,
    .mWidth = Spacing * 10,
  };
  static const Style offStyles  = baseStyles + Style {
    .mBackgroundColor = ToggleSwitchFillOff,
    .mBorderColor = ToggleSwitchStrokeOff,
    .mAnd = {
      {Disabled, Style {
        .mBackgroundColor = ToggleSwitchFillOffDisabled,
        .mBorderColor = ToggleSwitchStrokeOffDisabled,
      }},
      {Hover, Style {
        .mBackgroundColor = ToggleSwitchFillOffPointerOver,
      }},
      {Active, Style {
        .mBackgroundColor = ToggleSwitchFillOffPressed,
      }},
    },
  };
  static const Style onStyles = baseStyles + Style {
    .mBackgroundColor = ToggleSwitchFillOn,
    .mBorderColor = ToggleSwitchStrokeOn,
    .mAnd = {
      { Disabled , Style {
        .mBackgroundColor = ToggleSwitchFillOnDisabled,
        .mBorderColor = ToggleSwitchStrokeOnDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ToggleSwitchFillOnPointerOver,
        .mBorderColor = ToggleSwitchStrokeOnPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ToggleSwitchFillOnPressed,
        .mBorderColor = ToggleSwitchStrokeOnPressed,
      }},
    },
  };
  return (this->IsOn() ? onStyles : offStyles);
}

WidgetList ToggleSwitchKnob::GetDirectChildren() const noexcept {
  return {mThumb.get()};
}

}// namespace FredEmmott::GUI::Widgets
