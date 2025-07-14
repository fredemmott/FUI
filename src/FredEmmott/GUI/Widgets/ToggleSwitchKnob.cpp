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
  static const auto baseStyles
    = Style()
        .BackgroundColor(std::nullopt, ColorAnimation)
        .BorderColor(std::nullopt, ColorAnimation)
        .BorderRadius(Spacing * 2.5f)
        .BorderWidth(ToggleSwitchOuterBorderStrokeThickness)
        .FlexBasis(Spacing * 10)
        .FlexDirection(YGFlexDirectionColumn)
        .Height(20)
        .MarginRight(ToggleSwitchPreContentMargin)
        .Width(Spacing * 10);

  static const auto offStyles = baseStyles
    + Style()
        .BackgroundColor(ToggleSwitchFillOff)
        .BorderColor(ToggleSwitchStrokeOff)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ToggleSwitchFillOffDisabled)
            .BorderColor(ToggleSwitchStrokeOffDisabled))
        .And(Hover, Style().BackgroundColor(ToggleSwitchFillOffPointerOver))
        .And(Active, Style().BackgroundColor(ToggleSwitchFillOffPressed));

  static const auto onStyles = baseStyles
    + Style()
        .BackgroundColor(ToggleSwitchFillOn)
        .BorderColor(ToggleSwitchStrokeOn)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ToggleSwitchFillOnDisabled)
            .BorderColor(ToggleSwitchStrokeOnDisabled))
        .And(
          Hover,
          Style()
            .BackgroundColor(ToggleSwitchFillOnPointerOver)
            .BorderColor(ToggleSwitchStrokeOnPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(ToggleSwitchFillOnPressed)
            .BorderColor(ToggleSwitchStrokeOnPressed));

  return (this->IsOn() ? onStyles : offStyles);
}

WidgetList ToggleSwitchKnob::GetDirectChildren() const noexcept {
  return {mThumb.get()};
}

}// namespace FredEmmott::GUI::Widgets
