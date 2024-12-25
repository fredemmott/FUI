// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "LinearGradientBrush.hpp"

#include <include/core/SkMatrix.h>
#include <skia/effects/SkGradientShader.h>

namespace FredEmmott::GUI {

LinearGradientBrush::LinearGradientBrush(
  MappingMode mode,
  SkPoint start,
  SkPoint end,
  const std::vector<Stop>& stops)
  : mMappingMode(mode) {
  SkPoint ends[] = {start, end};
  std::vector<SkScalar> positions;
  std::vector<SkColor> colors;
  for (auto&& [pos, color]: stops) {
    positions.push_back(pos);
    colors.push_back(color);
  }
  mShader = SkGradientShader::MakeLinear(
    ends, colors.data(), positions.data(), stops.size(), SkTileMode::kClamp);
}
SkPaint LinearGradientBrush::GetPaint(const SkRect& rect) const {
  if (mMappingMode == MappingMode::Absolute) {
    SkPaint paint;
    paint.setShader(mShader);
    return paint;
  }

  auto m = SkMatrix::Translate(rect.x(), rect.y())
             .postScale(rect.width(), rect.height());
  SkPaint paint;
  paint.setShader(mShader->makeWithLocalMatrix(m));
  return paint;
}

}// namespace FredEmmott::GUI