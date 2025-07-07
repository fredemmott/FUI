// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/yoga.h>

#include <type_traits>

namespace FredEmmott::GUI::style_detail {

template <class T>
struct default_t {
  static constexpr auto value = std::nullopt;
};

template <>
struct default_t<float> {
  static constexpr float value {YGUndefined};
};

template <class T>
  requires std::is_scoped_enum_v<T>
struct default_t<T> {
  static constexpr T value {};
};

template <>
struct default_t<YGOverflow> {
  static constexpr YGOverflow value {YGOverflowVisible};
};

template <class T>
constexpr auto default_v = default_t<T>::value;

}// namespace FredEmmott::GUI::style_detail