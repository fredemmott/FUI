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
  static const Style lazyInit = Style()
                                  .Height(NormalHeight)
                                  .Left(Margin, NormalAnimation)
                                  .Position(YGPositionTypeAbsolute)
                                  .Top(Margin)
                                  .Width(NormalWidth);
  return lazyInit;
}

const Style& OuterOn() {
  static const Style lazyInit
    = OuterOff() + Style().Left(ParentWidth - ((2 * Margin) + NormalWidth));
  return lazyInit;
}

const Style& InnerBase() {
  static const Style lazyInit
    = Style()
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
  return lazyInit;
}

const Style& InnerOn() {
  static const Style lazyInit = InnerBase()
    + Style()
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
  return lazyInit;
}

const Style& InnerOff() {
  static const Style lazyInit = InnerBase()
    + Style()
        .BackgroundColor(ToggleSwitchKnobFillOff)
        .And(Disabled, Style().BackgroundColor(ToggleSwitchKnobFillOffDisabled))
        .And(Hover, Style().BackgroundColor(ToggleSwitchKnobFillOffPointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(ToggleSwitchKnobFillOffPressed)
            .MarginLeft(0)
            .MarginRight(NormalWidth - ActiveWidth));
  return lazyInit;
}
}// namespace Styles
}// namespace

ToggleSwitchThumb::ToggleSwitchThumb(const std::size_t id)
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
  this->BuiltInStyles() = mIsOn ? Styles::OuterOn() : Styles::OuterOff();
  mInner->BuiltInStyles() = mIsOn ? Styles::InnerOn() : Styles::InnerOff();
}

}// namespace FredEmmott::GUI::Widgets
