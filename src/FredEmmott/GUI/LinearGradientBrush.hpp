// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>
#include <skia/core/SkRect.h>
#include <skia/core/SkShader.h>

#include <Vector>

namespace FredEmmott::GUI {

class LinearGradientBrush final {
 public:
  enum class MappingMode {
    Absolute,
    RelativeToBoundingBox,
  };
  struct Stop {
    SkScalar mOffset;
    SkColor mColor;
  };

  LinearGradientBrush() = delete;

  LinearGradientBrush(
    MappingMode mode,
    SkPoint start,
    SkPoint end,
    const std::vector<Stop>& stops);

  [[nodiscard]] SkPaint GetPaint(const SkRect&) const;

  bool operator==(const LinearGradientBrush&) const = default;

 private:
  sk_sp<SkShader> mShader;
  MappingMode mMappingMode;
};

}// namespace FredEmmott::GUI