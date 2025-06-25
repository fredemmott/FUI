// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Font.hpp"

#include "Immediate/TextBlock.hpp"
#include "detail/font_detail.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFontTypes.h>
#endif

using namespace FredEmmott::GUI::font_detail;

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
  return Font {};
}

float Font::MeasureTextWidth(const std::string_view text) const noexcept {
  return renderer_detail::GetFontMetricsProvider()->MeasureTextWidth(
    *this, text);
}

}// namespace FredEmmott::GUI