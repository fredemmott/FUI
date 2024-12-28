// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>

#include "Theme.hpp"

namespace FredEmmott::GUI::StaticTheme {

template <class T>
struct Resource {
  T mDefault {};// a.k.a. 'dark', but also fallback for the others
  std::optional<T> mLight {};
  std::optional<T> mHighContrast {};

  constexpr T Resolve(const Theme theme = GetCurrent()) const noexcept {
    using enum Theme;
    switch (theme) {
      case Light:
        return mLight.value_or(mDefault);
      case Dark:
        return mDefault;
      case HighContrast:
        return mHighContrast.value_or(mDefault);
    }
    std::unreachable();
  };

  constexpr bool operator==(const Resource& other) const noexcept = default;
};

template <class T>
struct Resource<const Resource<T>*> {
  Resource<T>* mDefault {nullptr};
  Resource<T>* mLight {nullptr};
  Resource<T>* mHighContrast {nullptr};

  operator Resource<T>*() const noexcept {
    const auto theme = GetCurrent();
    using enum Theme;
    switch (theme) {
      case Light:
        if (mLight) {
          return mLight;
        }
        return mDefault;
      case Dark:
        return mDefault;
      case HighContrast:
        if (mHighContrast) {
          return mHighContrast;
        }
        return mDefault;
    }
    std::unreachable();
  }
};

}// namespace FredEmmott::GUI::StaticTheme
