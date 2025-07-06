// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>

#include "Theme.hpp"

namespace FredEmmott::GUI::StaticTheme {

template <class T>
struct Resource {
  using value_type = T;

  T mDefault {};// a.k.a. 'dark', but also fallback for the others
  std::optional<T> mLight {};
  std::optional<T> mHighContrast {};

  constexpr const T* Resolve(const Theme theme = GetCurrent()) const noexcept {
    using enum Theme;
    switch (theme) {
      case Light:
        return mLight.has_value() ? &mLight.value() : &mDefault;
      case Dark:
        return &mDefault;
      case HighContrast:
        return mHighContrast.has_value() ? &mHighContrast.value() : &mDefault;
    }
    std::unreachable();
  };

  constexpr bool operator==(const Resource& other) const noexcept = default;
};

}// namespace FredEmmott::GUI::StaticTheme
