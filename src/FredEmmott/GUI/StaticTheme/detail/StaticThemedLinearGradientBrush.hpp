// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkScalar.h>
#include <skia/core/SkPoint.h>
#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/LinearGradientBrush.hpp>

namespace FredEmmott::GUI::StaticTheme {

struct StaticThemedLinearGradientBrush {
  struct Stop {
    SkScalar mOffset;
    Color mColor;
  };
  StaticThemedLinearGradientBrush() = delete;
  StaticThemedLinearGradientBrush(
    LinearGradientBrush::MappingMode mode,
    SkPoint start,
    SkPoint end,
    const std::vector<Stop>& stops,
    ScaleTransform scaleTransform = {})
    : mMappingMode(mode), mStart(start), mEnd(end), mStops(stops), mScaleTransform(scaleTransform) {
    }
  template<StaticTheme::Theme TTheme>
  auto Resolve() const {
    std::vector<LinearGradientBrush::Stop> stops;
    stops.reserve(mStops.size());
    for (auto&& stop: mStops) {
      stops.push_back({stop.mOffset, stop.mColor.ResolveForStaticTheme<TTheme>()});
    }
    return LinearGradientBrush { mMappingMode, mStart, mEnd, stops, mScaleTransform };
  }
 private:
  LinearGradientBrush::MappingMode mMappingMode;
  SkPoint mStart;
  SkPoint mEnd;
  std::vector<Stop> mStops;
  ScaleTransform mScaleTransform;
};

}