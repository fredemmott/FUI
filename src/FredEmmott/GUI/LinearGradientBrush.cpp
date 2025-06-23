// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "LinearGradientBrush.hpp"

#include <stdexcept>

namespace FredEmmott::GUI {

LinearGradientBrush::LinearGradientBrush(
  MappingMode mode,
  const Point& start,
  const Point& end,
  const std::vector<Stop>& stops,
  ScaleTransform scaleTransform)
  : mMappingMode(mode),
    mStart(start),
    mEnd(end),
    mStops(stops),
    mScaleTransform(scaleTransform) {
  if (stops.size() < 2) [[unlikely]] {
    throw std::invalid_argument(
      "linear gradients must have at least two stops");
  }
}

}// namespace FredEmmott::GUI