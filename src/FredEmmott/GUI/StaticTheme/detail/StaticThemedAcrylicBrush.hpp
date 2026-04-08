// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/AcrylicBrush.hpp>
#include <FredEmmott/GUI/StaticTheme/Resource.hpp>

namespace FredEmmott::GUI::StaticTheme {

class StaticThemedAcrylicBrush {
 public:
  StaticThemedAcrylicBrush() = delete;
  template <class TTint, class TFallback>
  constexpr StaticThemedAcrylicBrush(
    const TTint& tint,
    const float opacity,
    const std::optional<float> luminosityOpacity,
    const TFallback& fallback)
    : mTint(MakeResource<Color>(tint)),
      mOpacity(opacity),
      mLuminosityOpacity(luminosityOpacity),
      mFallback(MakeResource<Color>(fallback)) {}

  [[nodiscard]]
  constexpr AcrylicBrush Resolve(const Theme theme) const {
    return AcrylicBrush {
      mTint.Resolve(theme).Resolve(),
      mOpacity,
      mLuminosityOpacity,
      mFallback.Resolve(theme).Resolve(),
    };
  }

 private:
  Resource<Color> mTint;
  float mOpacity;
  std::optional<float> mLuminosityOpacity;
  Resource<Color> mFallback;
};

}// namespace FredEmmott::GUI::StaticTheme