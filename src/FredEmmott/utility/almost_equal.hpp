// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <algorithm>
#include <concepts>
#include <limits>

namespace FredEmmott::utility {

template <std::floating_point T>
constexpr bool almost_equal(T a, T b) noexcept {
  if (a == 0 || b == 0) {
    return (std::abs(a - b) < std::numeric_limits<T>::epsilon());
  }
  if (std::signbit(a) != std::signbit(b)) {
    return false;
  }

  const auto diff = std::abs(a - b) / std::min(std::abs(a), std::abs(b));
  return diff <= std::numeric_limits<T>::epsilon();
}

}// namespace FredEmmott::utility