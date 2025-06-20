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
  float p0,
  float p1,
  float p2,
  float p3) {
  return {delay, duration, EasingFunctions::CubicBezier {p0, p1, p2, p3}};
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration duration,
  float p0,
  float p1,
  float p2,
  float p3) {
  return {
    StyleTransition::Duration::zero(),
    duration,
    EasingFunctions::CubicBezier {p0, p1, p2, p3}};
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration delay,
  const StyleTransition::Duration duration,
  const auto& fourPoints) {
  const auto [p0, p1, p2, p3] = fourPoints;
  return CubicBezierStyleTransition(delay, duration, p0, p1, p2, p3);
};

constexpr StyleTransition CubicBezierStyleTransition(
  const StyleTransition::Duration duration,
  const auto& fourPoints) {
  const auto [p0, p1, p2, p3] = fourPoints;
  return CubicBezierStyleTransition(
    StyleTransition::Duration::zero(), duration, p0, p1, p2, p3);
};

}// namespace FredEmmott::GUI