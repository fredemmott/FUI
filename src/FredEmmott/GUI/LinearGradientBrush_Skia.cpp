// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <skia/core/SkMatrix.h>
#include <skia/core/SkRect.h>
#include <skia/effects/SkGradientShader.h>

#include "LinearGradientBrush.hpp"

namespace FredEmmott::GUI {

SkPaint LinearGradientBrush::GetSkiaPaint(const SkRect& rect) const {
  const_cast<LinearGradientBrush*>(this)->InitializeSkiaShader();

  auto m = mSkiaScale;
  if (mMappingMode == MappingMode::RelativeToBoundingBox) {
    m.postScale(rect.width(), rect.height());
  }
  m.postTranslate(rect.x(), rect.y());
  if (mMappingMode == MappingMode::Absolute) {
    if (mScaleTransform.mScaleX < 0) {
      const auto brushWidth = mEnd.mX - mStart.mX;
      const float offset = rect.width() - brushWidth;
      m.postTranslate(offset, 0);
    }
    if (mScaleTransform.mScaleY < 0) {
      const auto brushHeight = mEnd.mY - mStart.mY;
      const float offset = rect.height() - brushHeight;
      m.postTranslate(0, offset);
    }
  }

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
    colors.push_back(color.as<SkColor>());
  }
 const auto xRange = (mEnd.mX - mStart.mX);
  const auto yRange = (mEnd.mY - mStart.mY);

  auto centerX = mScaleTransform.mOrigin.mX;
  auto centerY = mScaleTransform.mOrigin.mY;

  if (mMappingMode == MappingMode::Absolute) {
    centerX = mStart.mX + (centerX * xRange);
    centerY = mStart.mY + (centerY * yRange);
  } else {
    centerX *= xRange;
    centerY *= yRange;
  }

  mSkiaScale.setScale(
    mScaleTransform.mScaleX, mScaleTransform.mScaleY, centerX, centerY);
  const SkPoint ends[] = {{mStart.mX, mStart.mY}, {mEnd.mX, mEnd.mY}};

  mSkiaShader = SkGradientShader::MakeLinear(
    ends, colors.data(), positions.data(), mStops.size(), SkTileMode::kClamp);
}

}// namespace FredEmmott::GUI