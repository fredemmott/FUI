// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "assert.hpp"

namespace FredEmmott::GUI {

struct CornerRadius {
  constexpr CornerRadius() = default;

  constexpr CornerRadius(const float radius)
    : mTopLeft(radius),
      mTopRight(radius),
      mBottomRight(radius),
      mBottomLeft(radius) {}

  constexpr CornerRadius(
    const float topLeft,
    const float topRight,
    const float bottomRight,
    const float bottomLeft)
    : mIsUniform(
        topLeft == topRight && topRight == bottomRight
        && bottomRight == bottomLeft),
      mTopLeft(topLeft),
      mTopRight(topRight),
      mBottomRight(bottomRight),
      mBottomLeft(bottomLeft) {}

  [[nodiscard]]
  constexpr bool IsUniform() const noexcept {
    return mIsUniform;
  }

  [[nodiscard]]
  constexpr bool IsEmpty() const noexcept {
    static constexpr auto Epsilon = std::numeric_limits<float>::epsilon();
    if (mTopLeft >= Epsilon) {
      return false;
    }
    if (mIsUniform) {
      return true;
    }
    return (mTopRight < Epsilon) && (mBottomRight < Epsilon)
      && (mBottomLeft < Epsilon);
  }

  [[nodiscard]]
  constexpr float GetUniformValue() const {
    FUI_ASSERT(mIsUniform);
    return mTopLeft;
  }

  [[nodiscard]]
  constexpr float GetTopLeft() const noexcept {
    return mTopLeft;
  }
  [[nodiscard]]
  constexpr float GetTopRight() const noexcept {
    return mTopRight;
  }

  [[nodiscard]]
  constexpr float GetBottomRight() const noexcept {
    return mBottomRight;
  }

  [[nodiscard]]
  constexpr float GetBottomLeft() const noexcept {
    return mBottomLeft;
  }

  constexpr bool operator==(const CornerRadius&) const noexcept = default;

 private:
  bool mIsUniform {true};
  float mTopLeft {};
  float mTopRight {};
  float mBottomRight {};
  float mBottomLeft {};
};

struct UniformCornerRadius : CornerRadius {
  constexpr UniformCornerRadius() = default;
  constexpr UniformCornerRadius(const float radius) : CornerRadius(radius) {}

  constexpr operator float() const noexcept {
    return GetUniformValue();
  }

  using CornerRadius::operator==;
};

}// namespace FredEmmott::GUI