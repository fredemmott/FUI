// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <chrono>

#include "EasingFunctions.hpp"

namespace FredEmmott::GUI {

struct StyleTransition {
  using Duration = std::chrono::steady_clock::duration;
  Duration mDelay;
  Duration mDuration;
  EasingFunction mEasingFunction;

  constexpr bool operator==(const StyleTransition&) const noexcept = default;
};

constexpr auto InstantStyleTransition
  = StyleTransition {.mEasingFunction = EasingFunctions::Instant {}};

constexpr StyleTransition LinearStyleTransition(
  const StyleTransition::Duration delay,
  const StyleTransition::Duration duration) {
  return {delay, duration, EasingFunctions::Linear {}};
};

constexpr StyleTransition LinearStyleTransition(
  const StyleTransition::Duration duration) {
  return {
    StyleTransition::Duration::zero(), duration, EasingFunctions::Linear {}};
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration delay,
  const StyleTransition::Duration duration,
  float x1,
  float y1,
  float x2,
  float y2) {
  return {delay, duration, EasingFunctions::CubicBezier {x1, y1, x2, y2}};
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration duration,
  float x1,
  float y1,
  float x2,
  float y2) {
  return {
    StyleTransition::Duration::zero(),
    duration,
    EasingFunctions::CubicBezier {x1, y1, x2, y2}};
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration delay,
  const StyleTransition::Duration duration,
  const auto& x1y1x2y2) {
  const auto [x1, y1, x2, y2] = x1y1x2y2;
  return CubicBezierStyleTransition(delay, duration, x1, y1, x2, y2);
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration duration,
  const auto& x1y1x2y2) {
  const auto [x1, y1, x2, y2] = x1y1x2y2;
  return CubicBezierStyleTransition(
    StyleTransition::Duration::zero(), duration, x1, y1, x2, y2);
};

}// namespace FredEmmott::GUI