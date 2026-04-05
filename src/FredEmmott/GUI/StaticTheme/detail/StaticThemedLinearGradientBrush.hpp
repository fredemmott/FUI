// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/LinearGradientBrush.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/StaticTheme/detail/ResolveColor.hpp>

namespace FredEmmott::GUI::StaticTheme {

struct StaticThemedLinearGradientBrush {
  struct Stop {
    float mOffset;
    Resource<Color> mColor;
  };
  StaticThemedLinearGradientBrush() = delete;
  StaticThemedLinearGradientBrush(
    const std::string_view cacheKey,
    const LinearGradientBrush::MappingMode mode,
    const Point& start,
    const Point& end,
    const std::vector<Stop>& stops,
    const ScaleTransform scaleTransform = {})
    : mCacheKey(cacheKey),
      mMappingMode(mode),
      mStart(start),
      mEnd(end),
      mStops(stops),
      mScaleTransform(scaleTransform) {}

  [[nodiscard]]
  LinearGradientBrush Resolve(const StaticTheme::Theme theme) const {
    std::vector<LinearGradientBrush::Stop> stops;
    stops.reserve(mStops.size());
    for (auto&& stop: mStops) {
      stops.emplace_back(stop.mOffset, stop.mColor.Resolve(theme).Resolve());
    }
    return LinearGradientBrush {
      mCacheKey, mMappingMode, mStart, mEnd, stops, mScaleTransform};
  }

 private:
  std::string_view mCacheKey;
  LinearGradientBrush::MappingMode mMappingMode;
  Point mStart;
  Point mEnd;
  std::vector<Stop> mStops;
  ScaleTransform mScaleTransform;
};

class StaticThemedAcrylicBrush {
 public:
  StaticThemedAcrylicBrush() = delete;
  template <class TTint, class TFallback>
  constexpr StaticThemedAcrylicBrush(
    const TTint& tint,
    const float opacity,
    const TFallback& fallback)
    : mTint(MakeResource<Color>(tint)),
      mOpacity(opacity),
      mFallback(MakeResource<Color>(fallback)) {}

  [[nodiscard]]
  constexpr AcrylicBrush Resolve(const Theme theme) const {
    return AcrylicBrush {
      mTint.Resolve(theme).Resolve(),
      mOpacity,
      mFallback.Resolve(theme).Resolve(),
    };
  }

 private:
  Resource<Color> mTint;
  float mOpacity;
  Resource<Color> mFallback;
};

}// namespace FredEmmott::GUI::StaticTheme