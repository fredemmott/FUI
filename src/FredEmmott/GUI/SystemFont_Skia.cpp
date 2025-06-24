// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemFont.hpp"

#include <skia/core/SkFontMgr.h>
#include <skia/ports/SkTypeface_win.h>

#include "Font.hpp"
#include "detail/font_detail.hpp"

using namespace FredEmmott::GUI::font_detail;

namespace {

const auto gManager = SkFontMgr_New_DirectWrite();

auto LoadTypeface(const SkFontStyle& style, auto name, auto... fallbacks) {
  const auto ret = gManager->matchFamilyStyle(name, style);
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

const auto gRegularTypeface
  = LoadTypeface(SkFontStyle::Normal(), "Segoe UI Variable Text", "Segoe UI");
const auto gBodyStrongTypeface
  = LoadTypeface(SemiBold, "Segoe UI Variable Text", "Segoe UI");
const auto gCaptionTypeface
  = LoadTypeface(SkFontStyle::Normal(), "Segoe UI Variable Small");
const auto gDisplayTypeface
  = LoadTypeface(SemiBold, "Segoe UI Variable Display", "Segoe UI");
const auto gGlyphTypeface = LoadTypeface(
  SkFontStyle::Normal(),
  "Segoe Fluent Icons",
  "Segoe MDL2 Assets");

template <Height THeight>
SkFont FontFromTypeface(auto typeface) {
  return {
    typeface,
    PixelsToPoints(THeight),
  };
}

}// namespace

namespace FredEmmott::GUI::SystemFont {

const SkFont gCaption = FontFromTypeface<Height::Caption>(gCaptionTypeface);
const SkFont gBody = FontFromTypeface<Height::Body>(gRegularTypeface);
const SkFont gBodyStrong
  = FontFromTypeface<Height::BodyStrong>(gBodyStrongTypeface);
const SkFont gBodyLarge = FontFromTypeface<Height::BodyLarge>(gRegularTypeface);
const SkFont gSubtitle = FontFromTypeface<Height::Subtitle>(gDisplayTypeface);
const SkFont gTitle = FontFromTypeface<Height::Title>(gDisplayTypeface);
const SkFont gTitleLarge
  = FontFromTypeface<Height::TitleLarge>(gDisplayTypeface);
const SkFont gDisplay = FontFromTypeface<Height::Display>(gDisplayTypeface);

#define DEFINE_GLYPH_FONT(USAGE) \
  const SkFont gGlyph##USAGE = FontFromTypeface<Height::USAGE>(gGlyphTypeface);
FUI_SYSTEM_FONT_USAGES(DEFINE_GLYPH_FONT)
#undef DEFINE_GLYPH_FONT

Font Resolve(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X) \
  case Usage::X: \
    return g##X;
    FUI_SYSTEM_FONT_USAGES(USAGE_CASE)
#undef USAGE_CASE
  }
  std::unreachable();
}

Font ResolveGlyphFont(const Usage usage) noexcept {
  switch (usage) {
#define USAGE_CASE(X) \
  case Usage::X: \
    return gGlyph##X;
    FUI_SYSTEM_FONT_USAGES(USAGE_CASE)
#undef USAGE_CASE
  }
  std::unreachable();
}

sk_sp<SkFontMgr> GetFontManager() noexcept {
  return gManager;
}

}// namespace FredEmmott::GUI::SystemFont