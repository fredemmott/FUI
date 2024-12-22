// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemFont.hpp"

#include <Shlobj.h>
#include <ports/SkFontMgr_empty.h>
#include <skia/core/SkFontMgr.h>
#include <wil/resource.h>

#include <filesystem>

namespace {

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

template <FredEmmott::GUI::SystemFont::Height THeight>
consteval auto PixelsToPoints() {
  return (static_cast<SkScalar>(THeight) * 72) / USER_DEFAULT_SCREEN_DPI;
}

constexpr auto x = PixelsToPoints<::FredEmmott::GUI::SystemFont::Height::Body>();

template <FredEmmott::GUI::SystemFont::Height THeight>
SkFont RegularFont() {
  return {
    gRegularTypeface,
    PixelsToPoints<THeight>(),
  };
}

template <FredEmmott::GUI::SystemFont::Height THeight>
SkFont SemiboldFont() {
  return {
    gSemiboldTypeface,
    PixelsToPoints<THeight>(),
  };
}

}// namespace

namespace FredEmmott::GUI {

const SkFont SystemFont::Caption = RegularFont<Height::Caption>();
const SkFont SystemFont::Body = RegularFont<Height::Body>();
const SkFont SystemFont::BodyStrong = SemiboldFont<Height::BodyStrong>();
const SkFont SystemFont::BodyLarge = RegularFont<Height::BodyLarge>();
const SkFont SystemFont::Subtitle = SemiboldFont<Height::Subtitle>();
const SkFont SystemFont::Title = SemiboldFont<Height::Title>();
const SkFont SystemFont::TitleLarge = SemiboldFont<Height::TitleLarge>();
const SkFont SystemFont::Display = SemiboldFont<Height::Display>();

}// namespace FredEmmott::GUI