// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <skia/core/SkFontMgr.h>

#include "Font.hpp"
#include "SystemFont.hpp"
#include "assert.hpp"// FUI_DEBUGBREAK
#include "detail/font_detail.hpp"
#include "detail/system_font_detail.hpp"

#include <array>
#include <cstdio>

#ifdef _WIN32
#include <skia/ports/SkTypeface_win.h>
#else
#include <skia/ports/SkFontMgr_fontconfig.h>
#include <skia/ports/SkFontScanner_FreeType.h>
#endif

using namespace FredEmmott::GUI::font_detail;
using namespace FredEmmott::GUI::SystemFont;

namespace FredEmmott::GUI::SystemFont {
sk_sp<SkFontMgr> GetFontManager() noexcept {
#ifdef _WIN32
  static const auto ret = SkFontMgr_New_DirectWrite();
#else
  // nullptr FcConfig → Skia creates a default; FreeType is the scanner
  // that reads font metrics/shapes. SkFontMgr takes ownership of the
  // FcConfig and SkFontScanner.
  static const auto ret = SkFontMgr_New_FontConfig(
    /*fc=*/nullptr, SkFontScanner_Make_FreeType());
#endif
  return ret;
}

namespace {

// Walk a list of family names and return the first one Skia's font manager
// finds. If none match, log the names that were tried and try a generic
// "sans-serif" alias (fontconfig substitutes whatever the system has
// configured as its default sans-serif — DejaVu/Liberation/Cantarell
// depending on distro). If even that's unavailable, fall back to Skia's
// legacy default typeface so we always return *something* drawable rather
// than null — null typefaces produced silent rendering bugs (Skia's
// internal fallback typeface renders glyphs at unexpected widths, which
// then trips style-system invariants).
template <class... Names>
sk_sp<SkTypeface> LoadTypeface(const SkFontStyle& style, Names... names) {
  const std::array<const char*, sizeof...(Names)> arr {
    static_cast<const char*>(names)...,
  };
  for (const char* const n : arr) {
    if (auto t = GetFontManager()->matchFamilyStyle(n, style); t) {
      return t;
    }
  }

  std::fprintf(stderr, "[FUI] Font lookup failed; tried");
  for (const char* const n : arr) {
    std::fprintf(stderr, " \"%s\"", n);
  }
  std::fprintf(stderr, ". Falling back to fontconfig \"sans-serif\".\n");

  if (auto t = GetFontManager()->matchFamilyStyle("sans-serif", style); t) {
    return t;
  }

  std::fprintf(
    stderr,
    "[FUI] \"sans-serif\" also unavailable; using Skia's legacy default "
    "typeface. Text will render but layout invariants tied to specific "
    "fonts (FontIcon, etc.) may misbehave.\n");
  return GetFontManager()->legacyMakeTypeface(nullptr, style);
}

namespace FontStyle {
constexpr SkFontStyle Normal = SkFontStyle::Normal();
constexpr SkFontStyle SemiBold {
  SkiaFontWeight(FontWeight::SemiBold),
  SkFontStyle::kNormal_Width,
  SkFontStyle::kUpright_Slant,
};
}// namespace FontStyle

struct UsageTypefaces {
#define DEFINE_TYPEFACE(NAME, WEIGHT, ...) \
  const sk_sp<SkTypeface> NAME = LoadTypeface(FontStyle::WEIGHT, __VA_ARGS__);
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

Font ResolveSkiaFont(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X, TYPEFACE) \
  case Usage::X: \
    return GetUsageFonts().X;
    FUI_ENUM_SYSTEM_FONT_FONTS(USAGE_CASE)
#undef USAGE_CASE
  }
  if constexpr (Config::Debug) {
    FUI_DEBUGBREAK();
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
    FUI_DEBUGBREAK();
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::SystemFont
