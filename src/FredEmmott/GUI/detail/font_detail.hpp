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
