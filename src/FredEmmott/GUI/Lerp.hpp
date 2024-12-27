// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Brush.hpp"
#include "SolidColorBrush.hpp"

namespace FredEmmott::GUI {

template <class T>
concept trivially_interpolatable = requires(T a, T b) {
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
template <trivially_interpolatable T>
constexpr T Lerp(T start, T end, double ratio) noexcept {
  return start + ((end - start) * ratio);
}

constexpr Color
Lerp(const Color& startRef, const Color& endRef, double ratio) noexcept {
  constexpr auto& f = Lerp<float>;
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
Brush Lerp(const Brush& startRef, const Brush& endRef, double ratio);

/// A type where we are able to linearly interpolate between two values.
template <class T>
concept lerpable = requires(const T& a, const T& b) {
  { Lerp(a, b, 0.5) } -> std::convertible_to<T>;
};

static_assert(lerpable<SkScalar>);
static_assert(lerpable<Color>);
static_assert(lerpable<Brush>);

}// namespace FredEmmott::GUI