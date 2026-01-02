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
const auto ToggleSwitchThumbStyleClass = StyleClass::Make("ToggleSwitch/Thumb");

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

auto InnerBase() {
  return Style()
    .AlignSelf(YGAlignStretch)
    .BackgroundColor(std::nullopt, ColorAnimation)
    .BorderColor(std::nullopt, ColorAnimation)
    .BorderRadius(NormalHeight / 2, FasterAnimation)
    .FlexGrow(1)
    .Margin(std::nullopt, FasterAnimation)
    .And(
      Hover,
      Style()
        .BorderRadius(HoverHeight / 2)
        .MarginBottom(-HoverMarginDiff)
        .MarginLeft(-HoverMarginDiff)
        .MarginRight(-HoverMarginDiff)
        .MarginTop(-HoverMarginDiff));
}

auto InnerOn() {
  return Style()
    .BackgroundColor(ToggleSwitchKnobFillOn)
    .BorderColor(ToggleSwitchStrokeOn)
    .And(
      Disabled,
      Style()
        .BackgroundColor(ToggleSwitchKnobFillOnDisabled)
        .BorderColor(ToggleSwitchStrokeOnDisabled))
    .And(
      Hover,
      Style()
        .BackgroundColor(ToggleSwitchKnobFillOnPointerOver)
        .BorderColor(ToggleSwitchStrokeOnPointerOver))
    .And(
      Active,
      Style()
        .BackgroundColor(ToggleSwitchKnobFillOnPressed)
        .BorderColor(ToggleSwitchStrokeOnPressed)
        .MarginLeft(NormalWidth - ActiveWidth)
        .MarginRight(2 * -HoverMarginDiff));
}

auto InnerOff() {
  return Style()
    .BackgroundColor(ToggleSwitchKnobFillOff)
    .And(Disabled, Style().BackgroundColor(ToggleSwitchKnobFillOffDisabled))
    .And(Hover, Style().BackgroundColor(ToggleSwitchKnobFillOffPointerOver))
    .And(
      Active,
      Style()
        .BackgroundColor(ToggleSwitchKnobFillOffPressed)
        .MarginLeft(0)
        .MarginRight(NormalWidth - ActiveWidth));
}

const ImmutableStyle& Outer() {
  static const ImmutableStyle ret {
    Style()
      .Height(NormalHeight)
      .Left(Margin, NormalAnimation)
      .Position(YGPositionTypeAbsolute)
      .Top(Margin)
      .Width(NormalWidth)
      .And(Checked, Style().Left(ParentWidth - ((2 * Margin) + NormalWidth))),
  };
  return ret;
}

const ImmutableStyle& Inner() {
  static const ImmutableStyle ret {
    InnerBase().And(Checked, InnerOn()).And(!Checked, InnerOff()),
  };
  return ret;
}

}// namespace Styles

}// namespace

ToggleSwitchThumb::ToggleSwitchThumb(const std::size_t id)
  : Widget(id, ToggleSwitchThumbStyleClass, Styles::Outer()) {
  this->SetChildren(
    {mInner = new Widget(
       0, LiteralStyleClass {"ToggleSwitch/Thumb/Inner"}, Styles::Inner())});
}

}// namespace FredEmmott::GUI::Widgets
