// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <utility>

namespace FredEmmott::utility {

template <template <class...> class T, class... Args>
using drop_last_t = decltype([]<std::size_t... I>(std::index_sequence<I...>) {
  using tuple = std::tuple<Args...>;
  return std::type_identity<T<std::tuple_element_t<I, tuple>...>> {};
}(std::make_index_sequence<sizeof...(Args) - 1> {}))::type;

}// namespace FredEmmott::utility