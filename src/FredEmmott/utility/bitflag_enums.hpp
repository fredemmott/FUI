// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/type_traits/concepts.hpp>
#include <utility>

namespace FredEmmott::utility::adl {
template <concepts::scoped_enum T>
consteval bool is_bitflag_enum(std::type_identity<T>) {
  return false;
}
}// namespace FredEmmott::utility::adl

namespace FredEmmott::utility::detail {
template <concepts::scoped_enum T>
consteval bool is_bitflag_enum_helper() {
  using namespace adl;
  return is_bitflag_enum(std::type_identity<T> {});
}
}// namespace FredEmmott::utility::detail

namespace FredEmmott::utility {

// enable for your type by adding...
//
// ```
// consteval bool is_bitflag_enum(std::type_identity<YourType>) {
//   return true;
// }
// ```
//
// ...to the enclosing namespace
template <class T>
concept bitflag_enum = detail::is_bitflag_enum_helper<T>();

inline namespace bitflag_enums {
template <bitflag_enum T>
constexpr T operator~(T v) {
  return static_cast<T>(~std::to_underlying(v));
}

template <bitflag_enum T>
constexpr T operator&(T a, T b) {
  return static_cast<T>(std::to_underlying(a) & std::to_underlying(b));
}

template <bitflag_enum T>
constexpr T operator|(T a, T b) {
  return static_cast<T>(std::to_underlying(a) | std::to_underlying(b));
}

template <bitflag_enum T>
constexpr T operator^(T a, T b) {
  return static_cast<T>(std::to_underlying(a) ^ std::to_underlying(b));
}

template <bitflag_enum T>
constexpr T& operator&=(T& lhs, T rhs) {
  lhs = lhs & rhs;
  return lhs;
}

template <bitflag_enum T>
constexpr T& operator|=(T& lhs, T rhs) {
  lhs = lhs | rhs;
  return lhs;
}

template <bitflag_enum T>
constexpr T& operator^=(T& lhs, T rhs) {
  lhs = lhs ^ rhs;
  return lhs;
}

}// namespace bitflag_enums

}// namespace FredEmmott::utility

using namespace FredEmmott::utility::bitflag_enums;
