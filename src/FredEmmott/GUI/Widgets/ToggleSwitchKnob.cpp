// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "ToggleSwitchKnob.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ToggleSwitch.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "WidgetList.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass ToggleSwitchKnobStyleClass("ToggleSwitchKnob");

constexpr auto KnobWidth = 40.0f;
constexpr auto KnobHeight = 20.0f;

constexpr auto ThumbHeight = 12.0f;
constexpr auto ThumbHeightPointerOver = 14.0f;

constexpr auto ThumbWidth = ThumbHeight;
constexpr auto ThumbWidthPointerOver = ThumbHeightPointerOver;
constexpr auto ThumbWidthPressed = 17.0f;

constexpr auto ThumbMargin = 4.0f;

constexpr auto EasingFunction
  = EasingFunctions::CubicBezier(StaticTheme::ControlFastOutSlowInKeySpline);

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
        .Height(KnobHeight)
        .MarginRight(ToggleSwitchPreContentMargin)
        .Width(KnobWidth);

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

ToggleSwitchKnob::ToggleSwitchKnob(Window* const window)
  : Widget(window, ToggleSwitchKnobStyleClass, Styles()) {
  SetState(State::Normal);
}

ToggleSwitchKnob::~ToggleSwitchKnob() = default;

FrameRateRequirement ToggleSwitchKnob::GetFrameRateRequirement()
  const noexcept {
  const auto now = std::chrono::steady_clock::now();
  if (
    (mThumbHeight.mEndTime > now) || (mThumbWidth.mEndTime > now)
    || (mThumbCenter.mEndTime > now)) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return Widget::GetFrameRateRequirement();
}

void ToggleSwitchKnob::Tick(const std::chrono::steady_clock::time_point&) {
  if (IsDisabled()) {
    SetState(State::Disabled);
    return;
  }
  if (IsActive()) {
    SetState(State::Active);
    return;
  }
  if (IsHovered()) {
    SetState(State::Hovered);
    return;
  }
  SetState(State::Normal);
}

void ToggleSwitchKnob::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style&) const {
  // We only have to paint the thumb; the styles take care of the outer border
  // and overall fill

  const auto now = std::chrono::steady_clock::now();
  const auto width = mThumbWidth.Evaluate(EasingFunction, now);
  const auto height = mThumbHeight.Evaluate(EasingFunction, now);
  const auto center = mThumbCenter.Evaluate(EasingFunction, now);

  const auto left = center - (width / 2);
  const auto right = center + (width / 2);
  const UniformCornerRadius radii {height / 2};
  const auto brush = [](const State state, const bool isChecked) {
    switch (state) {
      using namespace StaticTheme::ToggleSwitch;
      case State::Disabled:
        return isChecked ? ToggleSwitchKnobFillOnDisabled
                         : ToggleSwitchKnobFillOffDisabled;
        ;
      case State::Normal:
        return isChecked ? ToggleSwitchKnobFillOn : ToggleSwitchKnobFillOff;
      case State::Hovered:
        return isChecked ? ToggleSwitchKnobFillOnPointerOver
                         : ToggleSwitchKnobFillOffPointerOver;
      case State::Active:
        return isChecked ? ToggleSwitchKnobFillOnPressed
                         : ToggleSwitchKnobFillOffPressed;
    }
    std::unreachable();
  }(get<0>(mState), IsChecked());

  const auto centerLine = rect.GetTop() + (rect.GetHeight() / 2.0f);
  renderer->FillRoundedRect(
    *brush->Resolve(),
    Rect {
      Point {
        rect.GetLeft() + left,
        centerLine - (height / 2.0f),
      },
      Size {
        right - left,
        height,
      },
    },
    radii);
}

void ToggleSwitchKnob::SetState(const State state) {
  using namespace StaticTheme;

  const auto checked = IsChecked();
  {
    const auto newState = std::tuple {state, checked};
    if (mState == newState) {
      return;
    }
    mState = newState;
  }

  const auto now = std::chrono::steady_clock::now();
  const auto normalEnd = now + ControlNormalAnimationDuration;
  const auto fasterEnd = now + ControlFasterAnimationDuration;

  constexpr auto offset = ThumbMargin + (ThumbWidth / 2.0f);
  constexpr auto offsetPressed
    = offset + ((ThumbWidthPressed - ThumbWidthPointerOver) / 2);

  switch (state) {
    case State::Disabled:
    case State::Normal: {
      mThumbHeight.TransitionTo(
        ThumbHeight, EasingFunction, now, now, fasterEnd);
      mThumbWidth.TransitionTo(ThumbWidth, EasingFunction, now, now, fasterEnd);
      const auto center = checked ? (KnobWidth - offset) : offset;
      mThumbCenter.TransitionTo(center, EasingFunction, now, now, normalEnd);
      break;
    }
    case State::Hovered: {
      mThumbHeight.TransitionTo(
        ThumbHeightPointerOver, EasingFunction, now, now, fasterEnd);
      mThumbWidth.TransitionTo(
        ThumbWidthPointerOver, EasingFunction, now, now, fasterEnd);

      const auto center = checked ? (KnobWidth - offset) : offset;
      mThumbCenter.TransitionTo(center, EasingFunction, now, now, normalEnd);
      break;
    }
    case State::Active:
      mThumbHeight.TransitionTo(
        ThumbHeightPointerOver, EasingFunction, now, now, fasterEnd);
      mThumbWidth.TransitionTo(
        ThumbWidthPressed, EasingFunction, now, now, fasterEnd);
      const auto center = checked ? (KnobWidth - offsetPressed) : offsetPressed;
      mThumbCenter.TransitionTo(center, EasingFunction, now, now, fasterEnd);
      break;
  }
}

}// namespace FredEmmott::GUI::Widgets
