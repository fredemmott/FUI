// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>

#include "Interpolation/CubicBezier.hpp"
#include "Interpolation/Linear.hpp"

namespace FredEmmott::GUI::EasingFunctions {

struct Linear {
  constexpr float operator()(float t) const {
    return t;
  }

  constexpr bool operator==(const Linear&) const noexcept {
    return true;
  }
};

struct CubicBezier {
  CubicBezier() = delete;
  constexpr CubicBezier(float p0, float p1, float p2, float p3)
    : p0(p0), p1(p1), p2(p2), p3(p3) {
  }

  explicit constexpr CubicBezier(auto points) {
    std::tie(p0, p1, p2, p3) = points;
  }

  constexpr float operator()(float t) const {
    return Interpolation::CubicBezier(p0, p1, p2, p3, t);
  }

  constexpr bool operator==(const CubicBezier&) const noexcept = default;

 private:
  float p0, p1, p2, p3;
};

/// The CSS 'ease' transition
template <Interpolation::lerpable T>
constexpr CubicBezier Ease {0.25, 0.1, 0.25, 1};

/// The CSS 'ease-in' transition
template <Interpolation::lerpable T>
constexpr CubicBezier EaseIn {0.42, 0, 1, 1};

/// The CSS 'ease-in' transition
template <Interpolation::lerpable T>
constexpr CubicBezier EaseOut = {0, 0, 0.58, 1};

/// The CSS 'ease-in-out' transition
template <Interpolation::lerpable T>
constexpr CubicBezier EaseInOut {0.42, 0, 0.58, 1};

}// namespace FredEmmott::GUI::EasingFunctions

namespace FredEmmott::GUI {

/** An easing function maps the range [0..1] to [0..1].
 *
 * Explicit functors are used instead of `std::function` and friends so that we
 * can provide an `operator==`.
 */
class EasingFunction {
 public:
  using variant_t
    = std::variant<EasingFunctions::Linear, EasingFunctions::CubicBezier>;

  EasingFunction() = delete;
  constexpr EasingFunction(const std::convertible_to<variant_t> auto& func)
    : mFunction(func) {
  }

  /// Evaluating the easing function, mapping [0..1] to [0..1]
  constexpr float operator()(float t) const {
    return std::visit(
      [&](const auto& func) -> float { return func(t); }, mFunction);
  }

  constexpr bool operator==(const EasingFunction&) const noexcept = default;

 private:
  variant_t mFunction;
};
}// namespace FredEmmott::GUI
