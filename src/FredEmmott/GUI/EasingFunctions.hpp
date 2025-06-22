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
  constexpr CubicBezier(float x1, float y1, float x2, float y2)
    : x1(x1),
      y1(y1),
      x2(x2),
      y2(y2) {
    if consteval {
      mSamples.emplace(CreateSamples(x1, x2));
    }
  }

  explicit constexpr CubicBezier(auto points) {
    std::tie(x1, y1, x2, y2) = points;
    if consteval {
      mSamples.emplace(CreateSamples(x1, x2));
    }
  }

  constexpr float operator()(const float x) const {
    if (!mSamples) {
      // mutable in `const` method as it's a cache
      mSamples = CreateSamples(x1, x2);
    }

    // Find t for x
    float low = 0;
    float high = 1;
    if (x < mSamples->front().mX) {
      high = mSamples->front().mT;
    } else if (x > mSamples->back().mX) {
      low = mSamples->back().mT;
    } else {
      uint8_t lowIdx = 0;
      uint8_t highIdx = std::size(*mSamples) - 1;
      while (lowIdx < highIdx) {
        uint8_t mid = (highIdx + lowIdx) / 2;
        const auto& sample = mSamples->at(mid);
        if (x < sample.mX) {
          highIdx = mid;
          high = sample.mT;
        } else {
          lowIdx = mid + 1;
          low = sample.mT;
        }
      }
    }

    float t = (high + low) / 2;
    float approximateX = Interpolation::CubicBezier(x1, x2, t);
    for (uint8_t rounds = 0; rounds < 8 && std::abs(x - approximateX) > 1e-5f;
         ++rounds) {
      ++rounds;
      if (x > approximateX) {
        low = t;
      } else {
        high = t;
      }
      t = (high + low) / 2;
      approximateX = Interpolation::CubicBezier(x1, x2, t);
    }

    // Calculate y given t
    return Interpolation::CubicBezier(y1, y2, t);
  }

  constexpr bool operator==(const CubicBezier&) const noexcept = default;

 private:
  // as 1 / 11 == 0.09, which is < 0.1, this means we will always have a
  // looked-up (t, x) tuple with x within 0.1 of the desired x
  static constexpr size_t SampleCount = 11;
  struct Sample {
    float mT {};
    float mX {};
    constexpr bool operator==(const Sample&) const noexcept = default;
  };
  using Samples = std::array<Sample, SampleCount>;
  mutable std::optional<Samples> mSamples {};

  float x1, y1, x2, y2;

  static constexpr Samples CreateSamples(const float x1, const float x2) {
    Samples samples;
    for (std::ptrdiff_t i = 0; i < SampleCount; ++i) {
      const auto t = (i + 1.f) / (SampleCount + 1);
      samples[i] = {
        .mT = t,
        .mX = Interpolation::CubicBezier(x1, x2, t),
      };
    }
    return samples;
  }
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
    : mFunction(func) {}

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
