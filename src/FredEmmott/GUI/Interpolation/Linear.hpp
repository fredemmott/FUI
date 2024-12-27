// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Brush.hpp>

namespace FredEmmott::GUI::Interpolation {

template <class T>
concept trivially_lerpable = requires(T a, T b) {
  { a + ((b - a) * 1.23) } -> std::convertible_to<T>;
};

/**
 * Linearly interpolates between two values based on a given ratio.
 *
 * @param start The starting value of the interpolation.
 * @param end The ending value of the interpolation.
 * @param ratio The ratio of interpolation, typically between 0.0 and 1.0.
 * @return The interpolated value between start and end, based on the given
 * ratio.
 */
template <trivially_lerpable T>
constexpr T Linear(T start, T end, float ratio) noexcept {
  return start + ((end - start) * ratio);
}

constexpr Color
Linear(const Color& startRef, const Color& endRef, float ratio) noexcept {
  constexpr auto& f = Linear<float>;
  const auto start = SkColor4f::FromColor(startRef);
  const auto end = SkColor4f::FromColor(endRef);
  return SkColor4f {
    .fR = f(start.fR, end.fR, ratio),
    .fG = f(start.fG, end.fG, ratio),
    .fB = f(start.fB, end.fB, ratio),
    .fA = f(start.fA, end.fA, ratio),
  }
    .toSkColor();
}

/** Linear interpolation between two `Brush`es.
 *
 * The brushes must both be `SolidColor` brushes; otherwise, it will return
 * the start brush if ratio is < 0.5, and the end brush otherwise
 */
Brush Linear(const Brush& startRef, const Brush& endRef, float ratio);

/// A type where we are able to linearly interpolate between two values.
template <class T>
concept lerpable = requires(const T& a, const T& b) {
  { Linear(a, b, 0.5) } -> std::convertible_to<T>;
};

static_assert(lerpable<SkScalar>);
static_assert(lerpable<Color>);
static_assert(lerpable<Brush>);

}// namespace FredEmmott::GUI::Interpolation