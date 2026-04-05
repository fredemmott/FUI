// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Color.hpp>

#include "Theme.hpp"

namespace FredEmmott::GUI::StaticTheme {

template <class T>
struct Resource {
  using value_type = T;

  T mDefault {};// a.k.a. 'dark', but also fallback for the others
  T mLight {};
  T mHighContrast {};

  [[nodiscard]]
  constexpr const T& Resolve(const Theme theme = GetCurrent()) const noexcept {
    using enum Theme;
    switch (theme) {
      case Light:
        return mLight;
      case Dark:
        return mDefault;
      case HighContrast:
        return mHighContrast;
    }
    std::unreachable();
  }

  constexpr Resource<Color> WithAlphaMultipliedBy(
    const float alpha) const noexcept
    requires std::same_as<T, Color>
  {
    return {
      .mDefault = mDefault.WithAlphaMultipliedBy(alpha),
      .mLight = mLight.WithAlphaMultipliedBy(alpha),
      .mHighContrast = mHighContrast.WithAlphaMultipliedBy(alpha),
    };
  }

  constexpr bool operator==(const Resource& other) const noexcept = default;
};

}// namespace FredEmmott::GUI::StaticTheme
