// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>
#include <skia/core/SkRect.h>

namespace FredEmmott::GUI {

class LinearGradientBrush final {
 public:
  [[nodiscard]] SkPaint GetPaint(const SkRect&) const {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    return paint;
  }

  constexpr bool operator==(const LinearGradientBrush&) const = default;
};

}// namespace FredEmmott::GUI