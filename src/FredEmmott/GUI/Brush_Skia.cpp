// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include <felly/overload.hpp>

#include "Brush.hpp"

namespace FredEmmott::GUI {

template <>
SkPaint Brush::as<SkPaint>(Renderer*, const Rect& rect) const {
  return std::visit(
    felly::overload {
      [](const SolidColorBrush& brush) {
        SkPaint paint;
        paint.setColor(brush.as<SkColor>());
        return paint;
      },
      [](const AcrylicBrush& brush) {
        SkPaint paint;
        paint.setColor(brush.GetTintColor().as<SkColor>());
        return paint;
      },
      [rect](const LinearGradientBrush& brush) {
        return brush.GetSkiaPaint(rect);
      }},
    mBrush);
}

}// namespace FredEmmott::GUI
