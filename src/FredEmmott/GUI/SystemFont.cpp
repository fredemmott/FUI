// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemFont.hpp"

#include <Shlobj.h>
#include <ports/SkFontMgr_empty.h>
#include <skia/core/SkFontMgr.h>
#include <wil/resource.h>

#include <filesystem>

#include "Font.hpp"
#include "detail/font_detail.hpp"

using namespace FredEmmott::GUI::font_detail;

#define FUI_SYSTEM_FONT_USAGES(X) \
  X(Caption) \
  X(Body) \
  X(BodyStrong) \
  X(BodyLarge) \
  X(Subtitle) \
  X(Title) \
  X(TitleLarge) \
  X(Display)

namespace {
// Values from
// https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography
enum class Height : uint16_t {
  Caption = 16,
  Body = 20,
  BodyStrong = 20,
  BodyLarge = 24,
  Subtitle = 28,
  Title = 36,
  TitleLarge = 52,
  Display = 92,
};

std::filesystem::path GetFontsPath() {
  wil::unique_hlocal_string ret;
  SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, std::out_ptr(ret));
  return std::wstring_view {ret.get()};
}

const auto gManager = SkFontMgr_New_Custom_Empty();

auto LoadTypeface(auto name) {
  return gManager->makeFromFile((GetFontsPath() / name).string().c_str());
}

const auto gRegularTypeface = LoadTypeface("segoeui.ttf");
const auto gSemiboldTypeface = LoadTypeface("seguisb.ttf");

template <Height THeight>
SkFont RegularFont() {
  return {
    gRegularTypeface,
    PixelsToPoints(THeight),
  };
}

template <Height THeight>
SkFont SemiboldFont() {
  return {
    gSemiboldTypeface,
    PixelsToPoints(THeight),
  };
}
}// namespace

namespace FredEmmott::GUI::SystemFont {

const SkFont gCaption = RegularFont<Height::Caption>();
const SkFont gBody = RegularFont<Height::Body>();
const SkFont gBodyStrong = SemiboldFont<Height::BodyStrong>();
const SkFont gBodyLarge = RegularFont<Height::BodyLarge>();
const SkFont gSubtitle = SemiboldFont<Height::Subtitle>();
const SkFont gTitle = SemiboldFont<Height::Title>();
const SkFont gTitleLarge = SemiboldFont<Height::TitleLarge>();
const SkFont gDisplay = SemiboldFont<Height::Display>();

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

}// namespace FredEmmott::GUI::SystemFont