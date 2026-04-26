// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//

#include "../Font.hpp"

#include <utility>

#include "../assert.hpp"
#include "../detail/font_detail.hpp"
#include "../detail/renderer_detail.hpp"
#include "FredEmmott/utility/almost_equal.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFontMgr.h>
#include <skia/core/SkFontTypes.h>
#endif

using namespace FredEmmott::GUI::font_detail;

namespace FredEmmott::GUI {

#ifdef FUI_ENABLE_SKIA
Font::Font(const SkFont& font) : mFont(font) {
  // Win32 inherited a `font == SkFont{}` → monostate down-conversion: a
  // sentinel for "no font". Don't do it on Linux. SystemFont's typeface
  // lookup hands us SkFont(nullptr, size) when the named family isn't
  // installed (e.g. Segoe Fluent Icons or Segoe UI Variable Small on a
  // box where fontconfig has no substitute). Skia happily draws such a
  // SkFont using its internal fallback typeface; turning it into
  // monostate instead made font.as<SkFont>() throw bad_variant_access
  // during DrawText. Empty Font remains reachable via Font's default
  // ctor (variant's default alternative is monostate).
  mMetrics = renderer_detail::GetFontMetricsProvider()->GetFontMetrics(*this);
}
#endif

Font Font::WithSize(const float pixels) const noexcept {
  if (utility::almost_equal(pixels, mMetrics.mSize)) {
    return *this;
  }
#ifdef FUI_ENABLE_SKIA
  if (const auto it = std::get_if<SkFont>(&mFont)) {
    auto ret = *it;
    ret.setSize(pixels);
    return Font(ret);
  }
#endif
  if (std::holds_alternative<std::monostate>(mFont)) {
    return {};
  }
  std::unreachable();
}

Font Font::WithWeight(const FontWeight weight) const noexcept {
#ifdef FUI_ENABLE_SKIA
  if (const auto it = std::get_if<SkFont>(&mFont)) {
    const auto oldTypeface = it->getTypeface();
    const auto currentStyle = oldTypeface->fontStyle();
    const SkFontStyle newStyle {
      SkiaFontWeight(weight),
      currentStyle.width(),
      currentStyle.slant(),
    };
    SkString familyName;
    oldTypeface->getFamilyName(&familyName);
    const auto newTypeface = SystemFont::GetFontManager()->matchFamilyStyle(
      familyName.c_str(), newStyle);
    return {SkFont {newTypeface, it->getSize()}};
  }
#endif
  if (std::holds_alternative<std::monostate>(mFont)) {
    return {};
  }
  std::unreachable();
}

float Font::MeasureTextWidth(const std::string_view text) const noexcept {
  if (std::holds_alternative<std::monostate>(mFont)) {
    return 0.0f;
  }
  return renderer_detail::GetFontMetricsProvider()->MeasureTextWidth(
    *this, text);
}

}// namespace FredEmmott::GUI
