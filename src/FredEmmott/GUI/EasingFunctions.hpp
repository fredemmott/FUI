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
    : mX1(x1),
      mY1(y1),
      mX2(x2),
      mY2(y2) {
    Initialize();
  }

  template <class T>
  explicit constexpr CubicBezier(T&& points) {
    std::tie(mX1, mY1, mX2, mY2) = std::forward<T>(points);
    Initialize();
  }

  constexpr float operator()(const float x) const {
    if (!mSamples) {
      // mutable in `const` method as it's a cache
      mSamples = CreateSamples(mX1, mX2);
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
      uint8_t highIdx = static_cast<uint8_t>(std::size(*mSamples) - 1);
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
    auto [approximateX, slope] = GetXValueAndSlope(t);
    for (uint8_t i = 0; i < 8 && std::abs(x - approximateX) > XAccuracy; ++i) {
      if (x > approximateX) {
        low = t;
      } else {
        high = t;
      }
      if (slope < 1e-6) {
        // Binary search
        t = (high + low) / 2;
      } else {
        // Newton's method
        t -= (approximateX - x) / slope;
      }
      std::tie(approximateX, slope) = GetXValueAndSlope(t);
    }

    // Calculate y given t
    return GetYValue_ILoveWin32(t);
  }

  constexpr bool operator==(const CubicBezier&) const noexcept = default;

 private:
  constexpr void Initialize() {
    if consteval {
      mSamples.emplace(CreateSamples(mX1, mX2));
    }

    const auto v3x1 = 3 * mX1;
    const auto v3x2 = 3 * mX2;
    const auto v6x1 = 6 * mX1;

    mXC1 = 1 - v3x2 + v3x1;
    mXC2 = v3x2 - v6x1;
    mXC3 = v3x1;

    const auto v3y1 = 3 * mY1;
    const auto v3y2 = 3 * mY2;
    const auto v6y1 = 6 * mY1;
    mYC1 = 1 - v3y2 + v3y1;
    mYC2 = v3y2 - v6y1;
    mYC3 = v3y1;
  }

  static constexpr float XAccuracy = 1e-6;
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

  float mX1, mY1, mX2, mY2;
  float mXC1, mXC2, mXC3;
  float mYC1, mYC2, mYC3;

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

  // Horner method to calculate both at the same time, using pre-computed
  // and shared coefficients
  constexpr std::tuple<float, float> GetXValueAndSlope(const float t) const {
    const auto c1t = mXC1 * t;
    const auto value = (((c1t + mXC2) * t) + mXC3) * t;
    const auto slope = (3 * c1t * t) + (2 * mXC2 * t) + mXC3;
    return {value, slope};
  }

  // wingdi.h defines a macro called `GetYValue`, so we can't use that as a
  // function name
  constexpr float GetYValue_ILoveWin32(const float t) const {
    const auto c1t = mYC1 * t;
    const auto value = (((c1t + mYC2) * t) + mYC3) * t;
    return value;
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
