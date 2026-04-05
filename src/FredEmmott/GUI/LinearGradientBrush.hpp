// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <concepts>
#include <memory>
#include <vector>

#include "Rect.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkMatrix.h>
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
    Color::Constant mColor;

    Stop() = delete;
    constexpr Stop(const float offset, const Color::Constant& color)
      : mOffset(offset),
        mColor(color) {}
    constexpr bool operator==(const Stop&) const noexcept = default;
  };
  using ScaleTransform = ScaleTransform;

  LinearGradientBrush() = delete;

  constexpr LinearGradientBrush(
    const std::string_view cacheKey,
    const MappingMode mode,
    const Point& start,
    const Point& end,
    const std::vector<Stop>& stops,
    const ScaleTransform scaleTransform = {})
    : mCacheKey(cacheKey),
      mMappingMode(mode),
      mStart(start),
      mEnd(end),
      mStops(stops),
      mScaleTransform(scaleTransform) {
    if (stops.size() < 2) [[unlikely]] {
      throw std::invalid_argument(
        "linear gradients must have at least two stops");
    }
  }
  ~LinearGradientBrush();

  constexpr bool operator==(const LinearGradientBrush&) const = default;

#ifdef FUI_ENABLE_SKIA
  [[nodiscard]] SkPaint GetSkiaPaint(const SkRect&) const;
#endif
#ifdef FUI_ENABLE_DIRECT2D
  [[nodiscard]] ID2D1Brush* GetDirect2DBrush(ID2D1RenderTarget*, const Rect&)
    const;
#endif
 private:
  std::string_view mCacheKey;
  MappingMode mMappingMode;
  Point mStart;
  Point mEnd;
  std::vector<Stop> mStops;
  ScaleTransform mScaleTransform {};

#ifdef FUI_ENABLE_SKIA
  struct SkiaCache {
    sk_sp<SkShader> mShader;
    SkMatrix mScaleMatrix {};
    bool operator==(const SkiaCache&) const noexcept = default;
  };
  std::optional<SkiaCache> mSkiaCache;

  void InitializeSkiaShader();
#endif
#ifdef FUI_ENABLE_DIRECT2D
  struct Direct2DCache {
    wil::com_ptr<ID2D1LinearGradientBrush> mBrush;
    D2D1::Matrix3x2F mScaleMatrix {};
  };

  Direct2DCache* GetDirect2DCache(ID2D1RenderTarget*) const;
#endif
};

}// namespace FredEmmott::GUI