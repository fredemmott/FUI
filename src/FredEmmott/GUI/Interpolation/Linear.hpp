// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Brush.hpp>

namespace FredEmmott::GUI {
class Font;
}

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
Linear(const Color& startRef, const Color& endRef, const float ratio) noexcept {
  constexpr auto& f = Linear<float>;
  const auto [r0, g0, b0, a0] = startRef.GetRGBAFTuple();
  const auto [r1, g1, b1, a1] = endRef.GetRGBAFTuple();
  return Color::Constant::FromRGBA128F(
    f(r0, r1, ratio), f(g0, g1, ratio), f(b0, b1, ratio), f(a0, a1, ratio));
}

/** Linear interpolation between two `Brush`es.
 *
 * The brushes must both be `SolidColor` brushes; otherwise, it will return
 * the start brush if ratio is < 0.5, and the end brush otherwise
 */
Brush Linear(const Brush& startRef, const Brush& endRef, float ratio);

// Linear interpolation between two `Font`s
Font Linear(const Font& startRef, const Font& endRef, float ratio);

/// A type where we are able to linearly interpolate between two values.
template <class T>
concept lerpable = requires(const T& a, const T& b) {
  { Linear(a, b, 0.5) } -> std::convertible_to<T>;
};

static_assert(lerpable<float>);
static_assert(lerpable<Color>);
static_assert(lerpable<Brush>);

}// namespace FredEmmott::GUI::Interpolation