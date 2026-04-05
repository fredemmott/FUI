// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "AcrylicBrush.hpp"

#include "SystemSettings.hpp"

namespace FredEmmott::GUI {

Color::Constant AcrylicBrush::Resolve() const {
  if (!SystemSettings::Get().IsTransparencyEnabled()) {
    return mFallback;
  }
  return mTint.WithAlphaMultipliedBy(mOpacity);
}

}// namespace FredEmmott::GUI