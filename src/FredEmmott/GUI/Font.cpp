// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Font.hpp"

#include "Immediate/TextBlock.hpp"
#include "detail/font_detail.hpp"

using namespace FredEmmott::GUI::font_detail;

namespace FredEmmott::GUI {

#ifdef FUI_ENABLE_SKIA
Font::Font(const SkFont& font) : mFont(font) {
  if (font == SkFont {}) {
    mFont = std::monostate {};
    return;
  }

  SkFontMetrics pt {};
  const auto lineSpacingPt = PointsToPixels(font.getMetrics(&pt));
  mMetrics = {
    .mSize = PointsToPixels(font.getSize()),
    .mLineSpacing = PointsToPixels(lineSpacingPt),
    .mDescent = PointsToPixels(pt.fDescent),
  };
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
#ifdef FUI_ENABLE_SKIA
  if (const auto it = std::get_if<SkFont>(&mFont)) {
    return PointsToPixels(
      it->measureText(text.data(), text.size(), SkTextEncoding::kUTF8));
  }
#endif
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  return std::numeric_limits<float>::quiet_NaN();
}

Font::operator SkFont() const noexcept {
  const auto it = std::get_if<SkFont>(&mFont);
  return it ? (*it) : SkFont {};
}

}// namespace FredEmmott::GUI