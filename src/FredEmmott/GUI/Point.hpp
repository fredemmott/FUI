// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <cstdint>

namespace FredEmmott::GUI {

template <class T>
  requires(std::integral<T> || std::floating_point<T>)
struct BasicPoint {
  T mX {};
  T mY {};

  template <class Self>
  auto operator+(this const Self& self, const BasicPoint& other) {
    return Self {self.mX + other.mX, self.mY + other.mY};
  }

  auto& operator+=(const BasicPoint& other) {
    mX += other.mX;
    mY += other.mY;
    return *this;
  }

  template <class Self>
  auto operator-(this const Self& self, const BasicPoint& other) {
    return Self {self.mX - other.mX, self.mY - other.mY};
  }

  auto& operator-=(const BasicPoint& other) {
    mX -= other.mX;
    mY -= other.mY;
    return *this;
  }

  template <class Self>
  auto operator*(this const Self& self, const T mult) {
    return Self {self.mX * mult, self.mY * mult};
  }

  constexpr bool operator==(const BasicPoint&) const noexcept = default;

  template <class Other>
  Other as() const noexcept {
    return Other {mX, mY};
  }
};

using Point = BasicPoint<float>;
using NativePoint = BasicPoint<int32_t>;

}// namespace FredEmmott::GUI