// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <utility>

#include "type_tag.hpp"

namespace FredEmmott::utility {

template <template <class...> class T, class... Args>
using drop_last_t =
  typename decltype([]<std::size_t... I>(std::index_sequence<I...>) {
    using tuple = std::tuple<Args...>;
    return type_tag<T<std::tuple_element_t<I, tuple>...>>;
  }(std::make_index_sequence<sizeof...(Args) - 1> {}))::type;

}// namespace FredEmmott::utility