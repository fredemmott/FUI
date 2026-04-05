// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/StaticTheme/Theme.hpp>

namespace FredEmmott::GUI::StaticTheme {
struct StaticThemedLinearGradientBrush;

template <class T>
concept resolvable
  = requires(StaticTheme::Theme theme, T value) { value.Resolve(theme); };

template <Theme TTheme, class T>
constexpr auto ResolveThemedValue(const T& value) {
  if constexpr (resolvable<T>) {
    return value.Resolve(TTheme);
  } else {
    return value;
  }
}

template <class T>
  requires(!resolvable<T>)
constexpr auto MakeResource(const T& x) {
  return Resource<T> {x, x, x};
}

template <class T>
constexpr auto MakeResource(const Resource<T>& x) {
  return x;
}

template <class T, class U>
static T ResolveAs(const Theme theme)
  requires requires {
    U::Resolve(theme);
    T {U::Resolve(theme)};
  }
{
  return T {U::Resolve(theme)};
}

}// namespace FredEmmott::GUI::StaticTheme