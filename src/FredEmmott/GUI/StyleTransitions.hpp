// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkScalar.h>

#include <array>
#include <chrono>

#include "Color.hpp"

namespace FredEmmott::GUI {

struct LinearStyleTransition {
  LinearStyleTransition() = delete;
  constexpr LinearStyleTransition(const std::chrono::milliseconds duration)
    : mDuration(duration) {
  }

  constexpr std::chrono::milliseconds GetDuration() const noexcept {
    return mDuration;
  }

  constexpr float Evaluate(const SkScalar normalizedX) const {
    return normalizedX;
  }

  constexpr bool operator==(const LinearStyleTransition&) const noexcept
    = default;

 private:
  std::chrono::milliseconds mDuration;
};

struct CubicBezierStyleTransition {
  CubicBezierStyleTransition() = delete;
  constexpr CubicBezierStyleTransition(
    const std::chrono::milliseconds duration,
    const SkScalar p0,
    const SkScalar p1,
    const SkScalar p2,
    const SkScalar p3)
    : mDuration(duration), mPoints({p0, p1, p2, p3}) {
  }

  constexpr CubicBezierStyleTransition(
    const std::chrono::milliseconds duration,
    const std::array<SkScalar, 4>& points)
    : mDuration(duration), mPoints(points) {
  }

  constexpr std::chrono::milliseconds GetDuration() const noexcept {
    return mDuration;
  }

  constexpr float Evaluate(const SkScalar normalizedX) const {
    const auto t = normalizedX;

    // B(t) = (1 - t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃

    const auto [P0, P1, P2, P3] = mPoints;
    const auto v1_t = 1 - t;
    const auto v1_t2 = v1_t * v1_t;
    const auto v1_t3 = v1_t * v1_t * v1_t;
    const auto t2 = t * t;
    const auto t3 = t * t * t;

    // B(t) = (1 - t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
    return (v1_t3 * P0) + (3 * v1_t2 * t * P1) + (3 * v1_t * t2 * P2)
      + (t3 * P3);
  }

  constexpr bool operator==(const CubicBezierStyleTransition&) const noexcept
    = default;

 private:
  std::chrono::milliseconds mDuration;
  std::array<SkScalar, 4> mPoints;
};

}// namespace FredEmmott::GUI