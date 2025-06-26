// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cmath>
#include <limits>

#ifdef FUI_ENABLE_DIRECT2D
#include <dwrite.h>
#include <wil/com.h>

#include <string>
#endif

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
enum class Size : uint16_t {
  Caption = 12,
  Body = 14,
  BodyStrong = 14,
  BodyLarge = 18,
  Subtitle = 20,
  Title = 28,
  TitleLarge = 40,
  Display = 68,
};

namespace FredEmmott::GUI::font_detail {
// Win32: this is the same as USER_DEFAULT_SCREEN_DPI
static constexpr auto BaselineDPI = 96;

template <auto ToDPI>
constexpr float PixelsToDPI(const auto pixels) {
  return (static_cast<float>(pixels) * ToDPI) / BaselineDPI;
}

constexpr float PixelsToPoints(const auto pixels) {
  return PixelsToDPI<72>(pixels);
}

template <auto FromDPI>
constexpr float PixelsFromDPI(const float points) {
  const auto raw = (points * BaselineDPI) / FromDPI;
  // std::round() *should* be constexpr in C++23, but as of
  // 2024-12-22, this is not yet implemented in MSVC
  if consteval {
    const auto diff
      = (0.5 + std::numeric_limits<float>::epsilon()) * ((raw > 0) ? 1 : -1);
    return static_cast<uint32_t>(raw + diff);
  } else {
    return std::round(raw);
  }
}

constexpr float PointsToPixels(const float points) {
  return PixelsFromDPI<72>(points);
}

static_assert(PointsToPixels(72) == 96);
static_assert(PixelsToPoints(96) == 72);
static_assert(PointsToPixels(71.9) == 96);
static_assert(PointsToPixels(72.1) == 96);
static_assert(PointsToPixels(72) == 96);
static_assert(PointsToPixels(73) == 97);

#ifdef FUI_ENABLE_DIRECT2D
struct DirectWriteFont {
  std::wstring mName;
  DWRITE_FONT_WEIGHT mWeight {DWRITE_FONT_WEIGHT_NORMAL};
  float mSize {};
  wil::com_ptr<IDWriteTextFormat> mTextFormat;

  constexpr bool operator==(const DirectWriteFont& other) const noexcept {
    return mName == other.mName && mWeight == other.mWeight
      && mSize == other.mSize;
  }
};
#endif

}// namespace FredEmmott::GUI::font_detail
