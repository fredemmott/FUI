// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <skia/core/SkFontMgr.h>
#include <skia/ports/SkTypeface_win.h>

#include "Font.hpp"
#include "SystemFont.hpp"
#include "detail/font_detail.hpp"

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

constexpr SkFontStyle SemiBold {
  SkFontStyle::kSemiBold_Weight,
  SkFontStyle::kNormal_Width,
  SkFontStyle::kUpright_Slant,
};

struct UsageTypefaces {
  const sk_sp<SkTypeface> Regular
    = LoadTypeface(SkFontStyle::Normal(), "Segoe UI Variable Text", "Segoe UI");
  const sk_sp<SkTypeface> BodyStrong
    = LoadTypeface(SemiBold, "Segoe UI Variable Text", "Segoe UI");
  const sk_sp<SkTypeface> Caption
    = LoadTypeface(SkFontStyle::Normal(), "Segoe UI Variable Small");
  const sk_sp<SkTypeface> Display
    = LoadTypeface(SemiBold, "Segoe UI Variable Display", "Segoe UI");
  const sk_sp<SkTypeface> Glyph = LoadTypeface(
    SkFontStyle::Normal(),
    "Segoe Fluent Icons",
    "Segoe MDL2 Assets");
};

const UsageTypefaces& GetUsageTypefaces() {
  static const UsageTypefaces ret;
  return ret;
}

struct UsageFonts {
 private:
  const UsageTypefaces Typefaces = GetUsageTypefaces();
  template <Height THeight>
  static SkFont Load(auto typeface) {
    return {
      typeface,
      PixelsToPoints(THeight),
    };
  }

 public:
  const SkFont Caption = Load<Height::Caption>(Typefaces.Caption);
  const SkFont Body = Load<Height::Body>(Typefaces.Regular);
  const SkFont BodyStrong = Load<Height::BodyStrong>(Typefaces.BodyStrong);
  const SkFont BodyLarge = Load<Height::BodyLarge>(Typefaces.Regular);
  const SkFont Subtitle = Load<Height::Subtitle>(Typefaces.Display);
  const SkFont Title = Load<Height::Title>(Typefaces.Display);
  const SkFont TitleLarge = Load<Height::TitleLarge>(Typefaces.Display);
  const SkFont Display = Load<Height::Display>(Typefaces.Display);

#define DEFINE_GLYPH_FONT(USAGE) \
  const SkFont Glyph##USAGE = Load<Height::USAGE>(Typefaces.Glyph);
  FUI_ENUM_SYSTEM_FONT_USAGES(DEFINE_GLYPH_FONT)
#undef DEFINE_GLYPH_FONT
};

const UsageFonts& GetUsageFonts() {
  static const UsageFonts ret;
  return ret;
}

}// namespace

namespace FredEmmott::GUI::SystemFont {

Font Resolve(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X) \
  case Usage::X: \
    return GetUsageFonts().X;
    FUI_ENUM_SYSTEM_FONT_USAGES(USAGE_CASE)
#undef USAGE_CASE
  }
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

Font ResolveGlyphFont(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X) \
  case Usage::X: \
    return GetUsageFonts().Glyph##X;
    FUI_ENUM_SYSTEM_FONT_USAGES(USAGE_CASE)
#undef USAGE_CASE
  }
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::SystemFont