// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkScalar.h>

#include <cmath>

namespace FredEmmott::GUI::font_detail {
// Win32: this is the same as USER_DEFAULT_SCREEN_DPI
static constexpr auto BaselineDPI = 96;

constexpr SkScalar PixelsToPoints(const auto pixels) {
  return (static_cast<SkScalar>(pixels) * 72) / BaselineDPI;
}

constexpr SkScalar PointsToPixels(const SkScalar points) {
  const auto raw = (points * BaselineDPI) / 72;
  // std::round() *should* be constexpr in C++23, but as of
  // 2024-12-22, this is not yet implemented in MSVC
  if (std::is_constant_evaluated()) {
    const auto diff
      = (0.5 + std::numeric_limits<SkScalar>::epsilon()) * ((raw > 0) ? 1 : -1);
    return static_cast<uint32_t>(raw + diff);
  } else {
    return std::round(raw);
  }
}
static_assert(PointsToPixels(72) == 96);
static_assert(PixelsToPoints(96) == 72);
static_assert(PointsToPixels(71.9) == 96);
static_assert(PointsToPixels(72.1) == 96);
static_assert(PointsToPixels(72) == 96);
static_assert(PointsToPixels(73) == 97);

}// namespace FredEmmott::GUI::font_detail