// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <cstdint>

#include "Size.hpp"

namespace FredEmmott::GUI {

template <class T>
  requires(std::integral<T> || std::floating_point<T>)
struct BasicPoint {
 private:
  template <class U>
  struct is_size_type_t : std::false_type {};

  template <std::same_as<T> U>
  struct is_size_type_t<U> : std::true_type {};

  template <std::integral U>
    requires std::unsigned_integral<T> && std::same_as<std::make_signed_t<T>, U>
  struct is_size_type_t<U> : std::true_type {};

  template <class U>
  static constexpr auto is_size_type_v = is_size_type_t<U>::value;

 public:
  using value_type = T;

  T mX {};
  T mY {};

  template <class Self>
  constexpr auto operator+(this const Self& self, const BasicPoint& other) {
    return Self {self.mX + other.mX, self.mY + other.mY};
  }

  constexpr auto& operator+=(const BasicPoint& other) {
    mX += other.mX;
    mY += other.mY;
    return *this;
  }

  template <class U>
    requires is_size_type_v<U>
  constexpr auto& operator+=(const BasicSize<U>& other) {
    mX = static_cast<T>(mX + other.mWidth);
    mY = static_cast<T>(mY + other.mHeight);
    return *this;
  }

  template <class Self, class U>
    requires is_size_type_v<U>
  constexpr auto operator+(this const Self& self, const BasicSize<U>& other) {
    auto ret = self;
    ret += other;
    return ret;
  }

  template <class Self>
  constexpr auto operator-(this const Self& self, const BasicPoint& other) {
    return Self {self.mX - other.mX, self.mY - other.mY};
  }

  constexpr auto& operator-=(const BasicPoint& other) {
    mX -= other.mX;
    mY -= other.mY;
    return *this;
  }

  template <class Self>
  constexpr auto operator*(this const Self& self, const T mult) {
    return Self {self.mX * mult, self.mY * mult};
  }

  constexpr bool operator==(const BasicPoint&) const noexcept = default;

  template <class Other>
  constexpr Other as() const noexcept {
    if constexpr (requires { typename Other::value_type; }) {
      using U = typename Other::value_type;
      return Other {static_cast<U>(mX), static_cast<U>(mY)};
    } else {
      return Other {mX, mY};
    }
  }
};

using Point = BasicPoint<float>;
using NativePoint = BasicPoint<int32_t>;

}// namespace FredEmmott::GUI