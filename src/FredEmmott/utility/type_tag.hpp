// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::utility {
template <class T>
struct type_tag_t {
  using type = T;
};
template <class T>
inline constexpr type_tag_t<T> type_tag {};

template <auto T>
struct value_tag_t {
  static constexpr auto value = T;
};
template <auto T>
inline constexpr value_tag_t<T> value_tag {};

}// namespace FredEmmott::utility