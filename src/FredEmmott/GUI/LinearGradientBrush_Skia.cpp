// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <include/core/SkMatrix.h>
#include <skia/effects/SkGradientShader.h>

#include "LinearGradientBrush.hpp"

namespace FredEmmott::GUI {

SkPaint LinearGradientBrush::GetSkiaPaint(const SkRect& rect) const {
  const_cast<LinearGradientBrush*>(this)->InitializeSkiaShader();

  auto m = SkMatrix::I();
  if (mMappingMode == MappingMode::RelativeToBoundingBox) {
    m.postScale(rect.width(), rect.height());
  }
  m.postTranslate(rect.x(), rect.y());

  SkPaint paint;
  paint.setShader(mSkiaShader->makeWithLocalMatrix(m));
  return paint;
}

void LinearGradientBrush::InitializeSkiaShader() {
  if (mSkiaShader) {
    return;
  }
  std::vector<float> positions;
  std::vector<SkColor> colors;
  for (auto&& [pos, color]: mStops) {
    positions.push_back(pos);
    colors.push_back(color);
  }

  const auto xRange = (mEnd.mX - mStart.mX);
  const auto yRange = (mEnd.mY - mStart.mY);

  auto m = SkMatrix::I();
  m.postScale(
    mScaleTransform.mScaleX,
    mScaleTransform.mScaleY,
    mScaleTransform.mOrigin.mX * xRange,
    mScaleTransform.mOrigin.mY * yRange);
  const SkPoint ends[] = {{mStart.mX, mStart.mY}, {mEnd.mX, mEnd.mY}};

  mSkiaShader = SkGradientShader::MakeLinear(
                  ends,
                  colors.data(),
                  positions.data(),
                  mStops.size(),
                  SkTileMode::kClamp)
                  ->makeWithLocalMatrix(m);
}

}// namespace FredEmmott::GUI