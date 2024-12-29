// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/Color.hpp>

namespace FredEmmott::GUI::StaticTheme{

template <class First, class... Rest>
struct ResourceSupertype {
  using value_type = First;
  using type = Resource<value_type>;
};
template<class... Rest>
struct ResourceSupertype<Color, Rest...> {
  using value_type = std::conditional_t<(std::same_as<Color, Rest> && ...), Color, Brush>;
  using type = Resource<value_type>;
};

}
