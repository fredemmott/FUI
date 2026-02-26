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
constexpr LiteralStyleClass ToggleSwitchKnobStyleClass("ToggleSwitchKnob");

auto MakeStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ToggleSwitch;
  using namespace PseudoClasses;

  constexpr auto ColorAnimation
    = LinearStyleTransition(ControlFasterAnimationDuration);
  const auto BaseStyles
    = Style()
        .BackgroundColor(std::nullopt, ColorAnimation)
        .BorderColor(std::nullopt, ColorAnimation)
        .BorderRadius(10)
        .BorderWidth(ToggleSwitchOuterBorderStrokeThickness)
        .FlexBasis(40)
        .FlexDirection(YGFlexDirectionColumn)
        .Height(20)
        .MarginRight(ToggleSwitchPreContentMargin)
        .Width(40);

  const auto OffStyles
    = Style()
        .BackgroundColor(ToggleSwitchFillOff)
        .BorderColor(ToggleSwitchStrokeOff)
        .And(
          Disabled,
          Style()
            .BackgroundColor(ToggleSwitchFillOffDisabled)
            .BorderColor(ToggleSwitchStrokeOffDisabled))
        .And(Hover, Style().BackgroundColor(ToggleSwitchFillOffPointerOver))
        .And(Active, Style().BackgroundColor(ToggleSwitchFillOffPressed));

  const auto OnStyles
    = Style()
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

  return BaseStyles + Style().And(Checked, OnStyles).And(!Checked, OffStyles);
}

auto& Styles() {
  static const ImmutableStyle ret {MakeStyles()};
  return ret;
}

}// namespace

using namespace widget_detail;

ToggleSwitchKnob::ToggleSwitchKnob(const id_type id)
  : Widget(id, ToggleSwitchKnobStyleClass, Styles()) {
  this->SetChildren({new ToggleSwitchThumb({})});
}

}// namespace FredEmmott::GUI::Widgets
