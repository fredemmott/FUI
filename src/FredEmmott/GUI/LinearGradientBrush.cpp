// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "LinearGradientBrush.hpp"

#include <include/core/SkMatrix.h>
#include <skia/effects/SkGradientShader.h>

#include <stdexcept>

namespace FredEmmott::GUI {

LinearGradientBrush::LinearGradientBrush(
  MappingMode mode,
  SkPoint start,
  SkPoint end,
  const std::vector<Stop>& stops,
  ScaleTransform scaleTransform)
  : mMappingMode(mode) {
  if (stops.size() < 2) [[unlikely]] {
    throw std::invalid_argument(
      "linear gradients must have at least two stops");
  }
  std::vector<SkScalar> positions;
  std::vector<SkColor> colors;
  for (auto&& [pos, color]: stops) {
    positions.push_back(pos);
    colors.push_back(color);
  }
  constexpr bool TestGradients = false;
  if constexpr (TestGradients) {
    colors.front() = SK_ColorRED;
    colors.back() = SK_ColorGREEN;
  }

  auto m = SkMatrix::I();
  const auto xRange = (end.x() - start.x());
  const auto yRange = (end.y() - start.y());
  m.postScale(
    scaleTransform.mScaleX,
    scaleTransform.mScaleY,
    scaleTransform.mOrigin.x() * xRange,
    scaleTransform.mOrigin.y() * yRange);

  const SkPoint ends[] = {start, end};
  mShader
    = SkGradientShader::MakeLinear(
        ends, colors.data(), positions.data(), stops.size(), SkTileMode::kClamp)
        ->makeWithLocalMatrix(m);
}

SkPaint LinearGradientBrush::GetPaint(const SkRect& rect) const {
  auto m = SkMatrix::I();
  if (mMappingMode == MappingMode::RelativeToBoundingBox) {
    m.postScale(rect.width(), rect.height());
  }
  m.postTranslate(rect.x(), rect.y());

  SkPaint paint;
  paint.setShader(mShader->makeWithLocalMatrix(m));
  return paint;
}

}// namespace FredEmmott::GUI