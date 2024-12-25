// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>
#include <skia/core/SkRect.h>
#include <skia/core/SkShader.h>

namespace FredEmmott::GUI {

class LinearGradientBrush final {
 public:
  LinearGradientBrush() = default;
  [[nodiscard]] SkPaint GetPaint(const SkRect&) const {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    return paint;
  }

  bool operator==(const LinearGradientBrush&) const = default;

 private:
  sk_sp<SkShader> mShader;
};

}// namespace FredEmmott::GUI