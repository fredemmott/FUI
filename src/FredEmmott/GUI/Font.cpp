// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Font.hpp"

namespace FredEmmott::GUI {

static SkScalar PointsToPixels(SkScalar points) {
  return std::round((points * USER_DEFAULT_SCREEN_DPI) / 72);
}

Font::Font(const SkFont& f): mFont(f), mMetricsInPixels(f) {}

SkScalar Font::GetMetricsInPixels(SkFontMetrics* metrics) const {
  if (metrics != nullptr) {
    *metrics = mMetricsInPixels.mMetrics;
  }
  return mMetricsInPixels.mLineSpacing;
}

Font::MetricsInPixels::MetricsInPixels(const SkFont& font) {
  SkFontMetrics points {};
  mLineSpacing = PointsToPixels(font.getMetrics(&points));
  mMetrics = {
    .fFlags = points.fFlags,
    .fTop = PointsToPixels(points.fTop),
    .fAscent = PointsToPixels(points.fAscent),
    .fDescent = PointsToPixels(points.fDescent),
    .fBottom = PointsToPixels(points.fBottom),
    .fLeading = PointsToPixels(points.fLeading),
    .fAvgCharWidth = PointsToPixels(points.fAvgCharWidth),
    .fMaxCharWidth = PointsToPixels(points.fMaxCharWidth),
    .fXMin = PointsToPixels(points.fXMin),
    .fXMax = PointsToPixels(points.fXMax),
    .fXHeight = PointsToPixels(points.fXHeight),
    .fCapHeight = PointsToPixels(points.fCapHeight),
    .fUnderlineThickness = PointsToPixels(points.fUnderlineThickness),
    .fUnderlinePosition = PointsToPixels(points.fUnderlinePosition),
    .fStrikeoutThickness = PointsToPixels(points.fStrikeoutThickness),
    .fStrikeoutPosition = PointsToPixels(points.fStrikeoutPosition),
  };
}

}// namespace FredEmmott::GUI