// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <vector>

#include "Rect.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkPaint.h>
#include <skia/core/SkShader.h>
#endif
#ifdef FUI_ENABLE_DIRECT2D
#include <d2d1_3.h>
#include <wil/com.h>
#endif

namespace FredEmmott::GUI {
struct ScaleTransform {
  Point mOrigin {0, 0};
  float mScaleX {1};
  float mScaleY {1};
  bool operator==(const ScaleTransform&) const noexcept = default;
};

class LinearGradientBrush final {
 public:
  enum class MappingMode {
    Absolute,
    RelativeToBoundingBox,
  };
  struct Stop {
    float mOffset {};
    Color mColor;

    Stop() = delete;
    constexpr Stop(float offset, const Color& color)
      : mOffset(offset),
        mColor(color) {}
    constexpr bool operator==(const Stop&) const noexcept = default;
  };
  using ScaleTransform = ScaleTransform;

  LinearGradientBrush() = delete;

  LinearGradientBrush(
    MappingMode mode,
    const Point& start,
    const Point& end,
    const std::vector<Stop>& stops,
    ScaleTransform scaleTransform = {});

  bool operator==(const LinearGradientBrush&) const = default;

#ifdef FUI_ENABLE_SKIA
  [[nodiscard]] SkPaint GetSkiaPaint(const SkRect&) const;
#endif
#ifdef FUI_ENABLE_DIRECT2D
  [[nodiscard]] wil::com_ptr<ID2D1Brush> GetDirect2DBrush(
    ID2D1RenderTarget*,
    const Rect&) const;
#endif
 private:
  MappingMode mMappingMode;
  Point mStart;
  Point mEnd;
  std::vector<Stop> mStops;
  ScaleTransform mScaleTransform {};

#ifdef FUI_ENABLE_SKIA
  sk_sp<SkShader> mSkiaShader;

  void InitializeSkiaShader();
#endif
#ifdef FUI_ENABLE_DIRECT2D
  wil::com_ptr<ID2D1GradientStopCollection> mDirect2DGradientStops;
  wil::com_ptr<ID2D1LinearGradientBrush> mDirect2DBrush;

  void InitializeDirect2DBrush(ID2D1RenderTarget*);
#endif
};

}// namespace FredEmmott::GUI