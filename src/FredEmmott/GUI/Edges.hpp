// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "assert.hpp"

namespace FredEmmott::GUI {
template <class T>
struct Edges {
  constexpr Edges() = default;
  constexpr Edges(const T value)
    : mIsUniform(true),
      mTop(value),
      mRight(value),
      mBottom(value),
      mLeft(value) {}

  constexpr Edges(const T vertical, const T horizontal)
    : mIsUniform(vertical == horizontal),
      mTop(vertical),
      mRight(horizontal),
      mBottom(vertical),
      mLeft(horizontal) {}

  constexpr Edges(const T top, const T right, const T bottom, const T left)
    : mIsUniform(top == right && right == bottom && bottom == left),
      mTop(top),
      mRight(right),
      mBottom(bottom),
      mLeft(left) {}

  [[nodiscard]]
  constexpr bool IsUniform() const noexcept {
    return mIsUniform;
  }

  [[nodiscard]]
  constexpr T GetUniformValue() const noexcept {
    FUI_ASSERT(mIsUniform);
    return mTop;
  }

  [[nodiscard]]
  constexpr T GetTop() const noexcept {
    return mTop;
  }

  [[nodiscard]]
  constexpr T GetRight() const noexcept {
    return mRight;
  }

  [[nodiscard]]
  constexpr T GetBottom() const noexcept {
    return mBottom;
  }

  [[nodiscard]]
  constexpr T GetLeft() const noexcept {
    return mLeft;
  }

  constexpr bool operator==(const Edges&) const noexcept = default;

 private:
  bool mIsUniform {false};
  const T mTop {};
  const T mRight {};
  const T mBottom {};
  const T mLeft {};
};

}// namespace FredEmmott::GUI