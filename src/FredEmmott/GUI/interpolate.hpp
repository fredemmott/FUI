// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Brush.hpp"

namespace FredEmmott::GUI {

template <class T>
concept trivially_interpolatable = requires(T a, T b) {
  { a + ((b - a) * 1.23) } -> std::convertible_to<T>;
};

template <class T>
struct interpolate_t;

template <trivially_interpolatable T>
T interpolate(T start, T end, double ratio) {
  return start + ((end - start) * ratio);
}

inline Color
interpolate(const Color& startRef, const Color& endRef, double ratio) {
  constexpr auto& f = interpolate<float>;
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

// Only functions for SolidColorBrushes
Brush interpolate(const Brush& startRef, const Brush& endRef, double ratio);

template <class T>
concept interpolatable = requires(const T& a, const T& b) {
  { interpolate(a, b, 0.5) } -> std::convertible_to<T>;
};

static_assert(interpolatable<SkScalar>);
static_assert(interpolatable<Color>);
static_assert(interpolatable<Brush>);

}// namespace FredEmmott::GUI