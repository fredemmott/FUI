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
  : mMappingMode(mode), mScaleTransform(scaleTransform) {
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
  constexpr bool TestGradients = true;
  if constexpr (false) {
    colors.front() = SK_ColorRED;
    colors.back() = SK_ColorGREEN;
  }

  const SkPoint ends[] = {start, end};
  mShader = SkGradientShader::MakeLinear(
    ends, colors.data(), positions.data(), stops.size(), SkTileMode::kClamp);
}

SkPaint LinearGradientBrush::GetPaint(const SkRect& rect) const {
  const auto& s = mScaleTransform;
  auto m = SkMatrix::Translate(-s.mOrigin.x(), -s.mOrigin.y())
             .postScale(s.mScaleX, s.mScaleY)
             .postTranslate(s.mOrigin.x(), s.mOrigin.y());
  if (mMappingMode == MappingMode::RelativeToBoundingBox) {
    m.postScale(rect.width(), rect.height());
  }

  m.postTranslate(rect.left(), rect.top());

  SkPaint paint;
  paint.setShader(mShader->makeWithLocalMatrix(m));
  return paint;
}

}// namespace FredEmmott::GUI