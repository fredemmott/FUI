// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>

#include "Linear.hpp"

namespace FredEmmott::GUI::Interpolation {

constexpr float CubicBezier(const float p1, const float p2, const float t) {
  /* B(t) = (1 - t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
   *
   * P₀ is always (0, 0) for 2D easing functions, and
   * P₃ is always (1, 1) for 2D easing functions, and
   * Given our p0 and p3 constants...
   *
   * B(t) = 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³
   */

  const auto v1_t = 1 - t;
  const auto v1_t2 = v1_t * v1_t;
  const auto t2 = t * t;
  const auto t3 = t * t * t;

  return (3 * v1_t2 * t * p1) + (3 * v1_t * t2 * p2) + t3;
}

}// namespace FredEmmott::GUI::Interpolation