// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <skia/core/SkFontMgr.h>
#include <skia/ports/SkTypeface_win.h>

#include "Font.hpp"
#include "SystemFont.hpp"
#include "detail/font_detail.hpp"
#include "detail/system_font_detail.hpp"

using namespace FredEmmott::GUI::font_detail;
using namespace FredEmmott::GUI::SystemFont;

namespace FredEmmott::GUI::SystemFont {
sk_sp<SkFontMgr> GetFontManager() noexcept {
  static const auto ret = SkFontMgr_New_DirectWrite();
  return ret;
}
}// namespace FredEmmott::GUI::SystemFont

namespace {

auto LoadTypeface(const SkFontStyle& style, auto name, auto... fallbacks) {
  const auto ret = GetFontManager()->matchFamilyStyle(name, style);
  if (ret) {
    return ret;
  }

  if constexpr (sizeof...(fallbacks) == 0) {
    return ret;
  } else {
    return LoadTypeface(style, fallbacks...);
  }
}

namespace FontWeight {
constexpr SkFontStyle Normal = SkFontStyle::Normal();
constexpr SkFontStyle SemiBold {
  SkFontStyle::kSemiBold_Weight,
  SkFontStyle::kNormal_Width,
  SkFontStyle::kUpright_Slant,
};
}// namespace FontWeight

struct UsageTypefaces {
#define DEFINE_TYPEFACE(NAME, WEIGHT, ...) \
  const sk_sp<SkTypeface> NAME = LoadTypeface(FontWeight::WEIGHT, __VA_ARGS__);
  FUI_ENUM_SYSTEM_FONT_TYPEFACES(DEFINE_TYPEFACE)
#undef DEFINE_TYPEFACE
};

const UsageTypefaces& GetUsageTypefaces() {
  static const UsageTypefaces ret;
  return ret;
}

struct UsageFonts {
 private:
  const UsageTypefaces Typefaces = GetUsageTypefaces();
  template <SystemFontSize TSize>
  static SkFont Load(auto typeface) {
    // The documentation lies: SkFont takes a font size in pixels/canvas units,
    // not points
    return {
      typeface,
      static_cast<float>(TSize),
    };
  }

 public:
#define DEFINE_FONT(USAGE, TYPEFACE) \
  const SkFont USAGE = Load<SystemFontSize::USAGE>(Typefaces.TYPEFACE);
  FUI_ENUM_SYSTEM_FONT_FONTS(DEFINE_FONT)
#undef DEFINE_FONT

#define DEFINE_GLYPH_FONT(USAGE, TYPEFACE) \
  const SkFont Glyph##USAGE = Load<SystemFontSize::USAGE>(Typefaces.Glyph);
  FUI_ENUM_SYSTEM_FONT_FONTS(DEFINE_GLYPH_FONT)
#undef DEFINE_GLYPH_FONT
};

const UsageFonts& GetUsageFonts() {
  static const UsageFonts ret;
  return ret;
}

}// namespace

namespace FredEmmott::GUI::SystemFont {

Font ResolveSkiaFont(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X, TYPEFACE) \
  case Usage::X: \
    return GetUsageFonts().X;
    FUI_ENUM_SYSTEM_FONT_FONTS(USAGE_CASE)
#undef USAGE_CASE
  }
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

Font ResolveGlyphSkiaFont(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X, TYPEFACE) \
  case Usage::X: \
    return GetUsageFonts().Glyph##X;
    FUI_ENUM_SYSTEM_FONT_FONTS(USAGE_CASE)
#undef USAGE_CASE
  }
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::SystemFont