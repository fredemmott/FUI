// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include <felly/overload.hpp>

#include "Brush.hpp"

namespace FredEmmott::GUI {

namespace {
template <class T>
concept as_color = requires(T& t) { t.template as<SkColor>(); };
}// namespace

template <>
SkPaint Brush::as<SkPaint>(Renderer*, const Rect& rect) const {
  return std::visit(
    felly::overload {
      [](const as_color auto& brush) {
        SkPaint paint;
        paint.setColor(brush.template as<SkColor>());
        return paint;
      },
      [rect](const LinearGradientBrush& brush) {
        return brush.GetSkiaPaint(rect);
      }},
    mBrush);
}

}// namespace FredEmmott::GUI
