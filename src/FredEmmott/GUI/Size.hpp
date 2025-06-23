// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <cstdint>

namespace FredEmmott::GUI {
template <class T>
  requires(std::integral<T> || std::floating_point<T>)
struct BasicSize {
  T mWidth {};
  T mHeight {};
};

using Size = BasicSize<float>;
}// namespace FredEmmott::GUI