// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkScalar.h>
#include <yoga/yoga.h>

namespace FredEmmott::GUI::style_detail {

template <class T>
struct default_t {
  static constexpr auto value = std::nullopt;
};

template <>
struct default_t<SkScalar> {
  static constexpr SkScalar value {YGUndefined};
};

template <class T>
constexpr auto default_v = default_t<T>::value;

}// namespace FredEmmott::GUI::style_detail