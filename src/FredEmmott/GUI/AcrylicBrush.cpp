// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "AcrylicBrush.hpp"

#include "SystemSettings.hpp"

namespace FredEmmott::GUI {

AcrylicBrush::AcrylicBrush(
  const Color& tint,
  const float opacity,
  const Color& fallback)
  : mTint(tint),
    mOpacity(opacity),
    mFallback(fallback) {
  // Don't pre-compute mTint * opacity, as the color may be a a system theme
  // color
}

Color AcrylicBrush::Resolve() const {
  if (!SystemSettings::Get().IsTransparencyEnabled()) {
    return mFallback;
  }
  return mTint.WithAlphaMultipliedBy(mOpacity);
}

}// namespace FredEmmott::GUI