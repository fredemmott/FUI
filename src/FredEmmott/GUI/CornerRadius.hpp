// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "assert.hpp"

namespace FredEmmott::GUI {

template <class T>
struct Corners {
  constexpr Corners() = default;

  constexpr Corners(const T radius)
    : mFlags(IsUniformBit | ConditionalBit<IsEmptyBit>(IsEmpty(radius))),
      mTopLeft(radius),
      mTopRight(radius),
      mBottomRight(radius),
      mBottomLeft(radius) {}

  constexpr Corners(
    const T topLeft,
    const T topRight,
    const T bottomRight,
    const T bottomLeft)
    : mFlags(
        ConditionalBit<IsUniformBit>(
          topLeft == topRight && topRight == bottomRight
          && bottomRight == bottomLeft)
        | ConditionalBit<IsEmptyBit>(
          IsEmpty(topLeft) && IsEmpty(topRight) && IsEmpty(bottomRight)
          && IsEmpty(bottomLeft))),
      mTopLeft(topLeft),
      mTopRight(topRight),
      mBottomRight(bottomRight),
      mBottomLeft(bottomLeft) {}

  [[nodiscard]]
  constexpr bool IsUniform() const noexcept {
    return (mFlags & IsUniformBit);
  }

  [[nodiscard]]
  constexpr bool IsEmpty() const noexcept {
    return (mFlags & IsEmptyBit);
  }

  [[nodiscard]]
  constexpr T GetUniformValue() const {
    FUI_ASSERT(IsUniform());
    return mTopLeft;
  }

  [[nodiscard]]
  constexpr T GetTopLeft() const noexcept {
    return mTopLeft;
  }
  [[nodiscard]]
  constexpr T GetTopRight() const noexcept {
    return mTopRight;
  }

  [[nodiscard]]
  constexpr T GetBottomRight() const noexcept {
    return mBottomRight;
  }

  [[nodiscard]]
  constexpr T GetBottomLeft() const noexcept {
    return mBottomLeft;
  }

  constexpr bool operator==(const Corners&) const noexcept = default;

 private:
  static constexpr auto Epsilon = std::numeric_limits<T>::epsilon();
  static constexpr uint8_t IsUniformBit = 0x01;
  static constexpr uint8_t IsEmptyBit = 0x02;

  template <uint8_t TBit>
  [[nodiscard]]
  static constexpr uint8_t ConditionalBit(const bool isSet) {
    return isSet ? TBit : 0;
  }

  [[nodiscard]]
  static constexpr bool IsEmpty(const T value) {
    return value > -Epsilon && value < Epsilon;
  }

  uint8_t mFlags {};
  T mTopLeft {};
  T mTopRight {};
  T mBottomRight {};
  T mBottomLeft {};
};

template <class T>
struct UniformCorners : Corners<T> {
  constexpr UniformCorners() = default;
  constexpr UniformCorners(const T radius) : Corners<T>(radius) {}

  constexpr operator T() const noexcept {
    return Corners<T>::GetUniformValue();
  }

  using Corners<T>::operator==;
};

using CornerRadius = Corners<float>;
using UniformCornerRadius = UniformCorners<float>;

}// namespace FredEmmott::GUI