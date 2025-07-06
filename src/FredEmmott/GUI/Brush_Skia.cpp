// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Brush.hpp"

namespace FredEmmott::GUI {

template <>
SkPaint Brush::as<SkPaint>(Renderer*, const Rect& rect) const {
  if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
    SkPaint paint;
    paint.setColor(it->as<SkColor>());
    return paint;
  }
  if (const auto it = get_if<LinearGradientBrush>(&mBrush)) {
    return it->GetSkiaPaint(rect);
  }
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI
