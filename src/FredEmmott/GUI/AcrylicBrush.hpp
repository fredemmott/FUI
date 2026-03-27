// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Color.hpp"

namespace FredEmmott::GUI {

class AcrylicBrush {
 public:
  AcrylicBrush() = delete;
  AcrylicBrush(const Color& tint, float opacity, const Color& fallback);

  Color Resolve() const;

  template <native_color T>
  T as() const noexcept {
    return Resolve().as<T>();
  }

  constexpr bool operator==(const AcrylicBrush&) const noexcept = default;

 private:
  Color mTint;
  float mOpacity;
  Color mFallback;
};

}// namespace FredEmmott::GUI