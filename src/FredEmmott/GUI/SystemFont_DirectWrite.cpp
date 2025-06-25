// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <dwrite.h>

#include "Font.hpp"
#include "SystemFont.hpp"
#include "detail/direct2d_detail.hpp"
#include "detail/direct_write_detail/DirectWriteFontProvider.hpp"
#include "detail/font_detail.hpp"
#include "detail/renderer_detail.hpp"
#include "detail/system_font_detail.hpp"

using namespace FredEmmott::GUI::font_detail;
using namespace FredEmmott::GUI::direct_write_detail;
using namespace FredEmmott::GUI::direct2d_detail;
using namespace FredEmmott::GUI::renderer_detail;
using namespace FredEmmott::GUI::win32_detail;

namespace FredEmmott::GUI::SystemFont {
namespace {

namespace FontWeight {
constexpr auto Normal = DWRITE_FONT_WEIGHT_NORMAL;
constexpr auto SemiBold = DWRITE_FONT_WEIGHT_SEMI_BOLD;
}// namespace FontWeight

std::wstring FindFontName(auto name, auto... fallbacks) {
  const auto collection
    = DirectWriteFontProvider::Get()->mSystemFontCollection.get();
  UINT32 index = 0;
  BOOL exists = false;
  const auto wideName = Utf8ToWide(name);
  if (
    FAILED(collection->FindFamilyName(wideName.c_str(), &index, &exists))
    || !exists) {
    if constexpr (sizeof...(fallbacks) == 0) {
      return {};
    } else {
      return FindFontName(fallbacks...);
    }
  }

  return wideName;
}
struct Typeface {
  DWRITE_FONT_WEIGHT mWeight {};
  std::wstring mName {};
};
}// namespace

struct UsageTypefaces {
#define DEFINE_TYPEFACE(NAME, WEIGHT, ...) \
  const Typeface NAME = {FontWeight::WEIGHT, FindFontName(__VA_ARGS__)};
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
  template <Height THeight>
  static DirectWriteFont Load(const Typeface& typeface) {
    const auto& [weight, name] = typeface;
    DirectWriteFont ret {
      .mName = typeface.mName,
      .mWeight = typeface.mWeight,
      .mSize = static_cast<float>(THeight),
    };
    CheckHResult(
      DirectWriteFontProvider::Get()->mDWriteFactory->CreateTextFormat(
        name.c_str(),
        nullptr,
        weight,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        PixelsToDIPs(THeight),
        L"",
        ret.mTextFormat.put()));
    return ret;
  }

 public:
#define DEFINE_FONT(USAGE, TYPEFACE) \
  const DirectWriteFont USAGE = Load<Height::USAGE>(Typefaces.TYPEFACE);
  FUI_ENUM_SYSTEM_FONT_FONTS(DEFINE_FONT)
#undef DEFINE_FONT

#define DEFINE_GLYPH_FONT(USAGE, TYPEFACE) \
  const DirectWriteFont Glyph##USAGE = Load<Height::USAGE>(Typefaces.Glyph);
  FUI_ENUM_SYSTEM_FONT_FONTS(DEFINE_GLYPH_FONT)
#undef DEFINE_GLYPH_FONT
};

const UsageFonts& GetUsageFonts() {
  static const UsageFonts ret;
  return ret;
}

}// namespace FredEmmott::GUI::SystemFont

namespace FredEmmott::GUI::SystemFont {

Font ResolveDirectWriteFont(const Usage usage) noexcept {
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

Font ResolveGlyphDirectWriteFont(const Usage usage) noexcept {
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