// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Color.hpp>
#include <FredEmmott/GUI/LinearGradientBrush.hpp>
#include <FredEmmott/GUI/Point.hpp>

namespace FredEmmott::GUI::StaticTheme {

struct StaticThemedLinearGradientBrush {
  struct Stop {
    float mOffset;
    Color mColor;
  };
  StaticThemedLinearGradientBrush() = delete;
  StaticThemedLinearGradientBrush(
    LinearGradientBrush::MappingMode mode,
    const Point& start,
    const Point& end,
    const std::vector<Stop>& stops,
    ScaleTransform scaleTransform = {})
    : mMappingMode(mode),
      mStart(start),
      mEnd(end),
      mStops(stops),
      mScaleTransform(scaleTransform) {}
  template <StaticTheme::Theme TTheme>
  auto Resolve() const {
    std::vector<LinearGradientBrush::Stop> stops;
    stops.reserve(mStops.size());
    for (auto&& stop: mStops) {
      stops.emplace_back(
        stop.mOffset, stop.mColor.ResolveForStaticTheme<TTheme>());
    }
    return LinearGradientBrush {
      mMappingMode, mStart, mEnd, stops, mScaleTransform};
  }

 private:
  LinearGradientBrush::MappingMode mMappingMode;
  Point mStart;
  Point mEnd;
  std::vector<Stop> mStops;
  ScaleTransform mScaleTransform;
};

}// namespace FredEmmott::GUI::StaticTheme