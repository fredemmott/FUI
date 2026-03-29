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
    const std::optional<float> luminosityOpacity,
    const Color::Constant& fallback)
    : mTint(tint),
      mOpacity(opacity),
      mLuminosityOpacity(luminosityOpacity),
      mFallback(fallback) {}

  [[nodiscard]]
  Color::Constant GetTintColor() const;

  [[nodiscard]]
  float GetTintOpacity() const;

  [[nodiscard]]
  std::optional<float> GetLuminosityOpacity() const noexcept {
    return mLuminosityOpacity;
  }

  [[nodiscard]]
  Color GetFallbackColor() const noexcept {
    return mFallback;
  }

  constexpr bool operator==(const AcrylicBrush&) const noexcept = default;

 private:
  Color::Constant mTint;
  float mOpacity;
  std::optional<float> mLuminosityOpacity;
  Color::Constant mFallback;
};

}// namespace FredEmmott::GUI
