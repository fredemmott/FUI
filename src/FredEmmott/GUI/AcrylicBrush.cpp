// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "AcrylicBrush.hpp"

#include "SystemSettings.hpp"

namespace FredEmmott::GUI {

Color::Constant AcrylicBrush::GetTintColor() const {
  if (SystemSettings::Get().IsTransparencyEnabled()) {
    return mTint;
  }
  return mFallback;
}

float AcrylicBrush::GetTintOpacity() const {
  if (SystemSettings::Get().IsTransparencyEnabled()) {
    return mOpacity;
  }
  return 1.0f;
}

}// namespace FredEmmott::GUI
