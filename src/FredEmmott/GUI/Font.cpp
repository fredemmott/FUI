// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Font.hpp"

#include "Immediate/TextBlock.hpp"
#include "assert.hpp"
#include "detail/font_detail.hpp"
#include "detail/win32_detail.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFontTypes.h>
#endif

#ifdef FUI_ENABLE_DIRECT2D
#include <FredEmmott/GUI/detail/direct_write_detail/DirectWriteFontProvider.hpp>
#endif

using namespace FredEmmott::GUI::font_detail;
using namespace FredEmmott::GUI::win32_detail;

namespace FredEmmott::GUI {

#ifdef FUI_ENABLE_SKIA
Font::Font(const SkFont& font) : mFont(font) {
  if (font == SkFont {}) {
    mFont = std::monostate {};
    return;
  }

  mMetrics = renderer_detail::GetFontMetricsProvider()->GetFontMetrics(*this);
}
#endif

#ifdef FUI_ENABLE_DIRECT2D
Font::Font(const DirectWriteFont& font) : mFont(font) {
  if (font == DirectWriteFont {}) {
    mFont = std::monostate {};
    return;
  }
  FUI_ASSERT(font.mSize > std::numeric_limits<float>::epsilon());

  mMetrics = renderer_detail::GetFontMetricsProvider()->GetFontMetrics(*this);
}
#endif

Font Font::WithSize(float pixels) const noexcept {
#ifdef FUI_ENABLE_SKIA
  if (const auto it = std::get_if<SkFont>(&mFont)) {
    auto ret = *it;
    ret.setSize(PixelsToPoints(pixels));
    return Font(ret);
  }
#endif
#ifdef FUI_ENABLE_DIRECT2D
  if (const auto it = std::get_if<DirectWriteFont>(&mFont)) {
    auto dup = *it;
    dup.mSize = pixels;
    dup.mTextFormat.reset();
    CheckHResult(
      direct_write_detail::DirectWriteFontProvider::Get()
        ->mDWriteFactory->CreateTextFormat(
          dup.mName.c_str(),
          nullptr,
          dup.mWeight,
          DWRITE_FONT_STYLE_NORMAL,
          DWRITE_FONT_STRETCH_NORMAL,
          dup.mSize,
          L"",
          dup.mTextFormat.put()));
    return {dup};
  }
#endif
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

float Font::MeasureTextWidth(const std::string_view text) const noexcept {
  return renderer_detail::GetFontMetricsProvider()->MeasureTextWidth(
    *this, text);
}

}// namespace FredEmmott::GUI