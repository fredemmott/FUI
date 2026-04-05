// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Color.hpp"

namespace FredEmmott::GUI {

class AcrylicBrush {
 public:
  AcrylicBrush() = delete;
  constexpr AcrylicBrush(
    const Color::Constant& tint,
    const float opacity,
    const Color::Constant& fallback)
    : mTint(tint),
      mOpacity(opacity),
      mFallback(fallback) {}

  [[nodiscard]]
  Color::Constant Resolve() const;

  constexpr bool operator==(const AcrylicBrush&) const noexcept = default;

 private:
  Color::Constant mTint;
  float mOpacity;
  Color::Constant mFallback;
};

}// namespace FredEmmott::GUI