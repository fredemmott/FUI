// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cmath>
#include <limits>

#define FUI_ENUM_SYSTEM_FONT_USAGES(X) \
  X(Caption) \
  X(Body) \
  X(BodyStrong) \
  X(BodyLarge) \
  X(Subtitle) \
  X(Title) \
  X(TitleLarge) \
  X(Display)

// Values from
// https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography
enum class Height : uint16_t {
  Caption = 16,
  Body = 20,
  BodyStrong = 20,
  BodyLarge = 24,
  Subtitle = 28,
  Title = 36,
  TitleLarge = 52,
  Display = 92,
};

namespace FredEmmott::GUI::font_detail {
// Win32: this is the same as USER_DEFAULT_SCREEN_DPI
static constexpr auto BaselineDPI = 96;

constexpr float PixelsToPoints(const auto pixels) {
  return (static_cast<float>(pixels) * 72) / BaselineDPI;
}

constexpr float PointsToPixels(const float points) {
  const auto raw = (points * BaselineDPI) / 72;
  // std::round() *should* be constexpr in C++23, but as of
  // 2024-12-22, this is not yet implemented in MSVC
  if (std::is_constant_evaluated()) {
    const auto diff
      = (0.5 + std::numeric_limits<float>::epsilon()) * ((raw > 0) ? 1 : -1);
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
