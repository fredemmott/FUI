// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>
#include <skia/core/SkRect.h>
#include <skia/core/SkShader.h>

#include <concepts>
#include <vector>

namespace FredEmmott::GUI {
struct ScaleTransform {
  SkPoint mOrigin {0, 0};
  SkScalar mScaleX {1};
  SkScalar mScaleY {1};
  bool operator==(const ScaleTransform&) const noexcept = default;
};

class LinearGradientBrush final {
 public:
  enum class MappingMode {
    Absolute,
    RelativeToBoundingBox,
  };
  struct Stop {
    SkScalar mOffset {};
    SkColor mColor {};

    Stop(SkScalar offset, const std::convertible_to<SkColor> auto& color)
      : mOffset(offset), mColor(color) {
    }
  };
  using ScaleTransform = ScaleTransform;

  LinearGradientBrush() = delete;

  LinearGradientBrush(
    MappingMode mode,
    SkPoint start,
    SkPoint end,
    const std::vector<Stop>& stops,
    ScaleTransform scaleTransform = {});

  [[nodiscard]] SkPaint GetPaint(const SkRect&) const;

  bool operator==(const LinearGradientBrush&) const = default;

 private:
  sk_sp<SkShader> mShader;
  MappingMode mMappingMode;
};

}// namespace FredEmmott::GUI