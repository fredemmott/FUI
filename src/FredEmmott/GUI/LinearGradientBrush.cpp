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
  const std::vector<Stop>& stops)
  : mMappingMode(mode) {
  if (stops.size() < 2) [[unlikely]] {
    throw std::invalid_argument(
      "linear gradients must have at least two stops");
  }
  SkPoint ends[] = {start, end};
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
  mShader = SkGradientShader::MakeLinear(
    ends, colors.data(), positions.data(), stops.size(), SkTileMode::kClamp);
}
SkPaint LinearGradientBrush::GetPaint(const SkRect& rect) const {
  auto m = (mMappingMode == MappingMode::Absolute)
    ? SkMatrix::I()
    : SkMatrix::Scale(rect.width(), rect.height());

  m = m.postTranslate(rect.left(), rect.top());

  SkPaint paint;
  paint.setShader(mShader->makeWithLocalMatrix(m));
  return paint;
}

}// namespace FredEmmott::GUI