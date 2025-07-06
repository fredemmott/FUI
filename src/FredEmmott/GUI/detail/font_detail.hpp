// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cmath>
#include <limits>

#include "FredEmmott/GUI/FontWeight.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFontStyle.h>
#endif
#ifdef FUI_ENABLE_DIRECT2D
#include <dwrite.h>
#include <wil/com.h>

#include <string>
#endif

namespace FredEmmott::GUI::font_detail {
// Win32: this is the same as USER_DEFAULT_SCREEN_DPI
static constexpr auto BaselineDPI = 96;

#ifdef FUI_ENABLE_SKIA
constexpr SkFontStyle::Weight SkiaFontWeight(const FontWeight v) {
  using enum FontWeight;
  switch (v) {
    case Normal:
      return SkFontStyle::kNormal_Weight;
    case SemiBold:
      return SkFontStyle::kSemiBold_Weight;
  }
  std::unreachable();
}
#endif

#ifdef FUI_ENABLE_DIRECT2D
constexpr DWRITE_FONT_WEIGHT DirectWriteFontWeight(const FontWeight v) {
  using enum FontWeight;
  switch (v) {
    case Normal:
      return DWRITE_FONT_WEIGHT_NORMAL;
    case SemiBold:
      return DWRITE_FONT_WEIGHT_SEMI_BOLD;
  }
  std::unreachable();
}

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
