// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SystemFont.hpp"

#include <Shlobj.h>
#include <ports/SkFontMgr_empty.h>
#include <skia/core/SkFontMgr.h>

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

constexpr auto x
  = PixelsToPoints<::FredEmmott::GUI::SystemFont::Height::Body>();

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

namespace FredEmmott::GUI::SystemFont {

const SkFont Caption = RegularFont<Height::Caption>();
const SkFont Body = RegularFont<Height::Body>();
const SkFont BodyStrong = SemiboldFont<Height::BodyStrong>();
const SkFont BodyLarge = RegularFont<Height::BodyLarge>();
const SkFont Subtitle = SemiboldFont<Height::Subtitle>();
const SkFont Title = SemiboldFont<Height::Title>();
const SkFont TitleLarge = SemiboldFont<Height::TitleLarge>();
const SkFont Display = SemiboldFont<Height::Display>();

const SkFont& Get(const Usage usage) noexcept {
  switch (usage) {
    case Usage::Caption:
      return Caption;
    case Usage::Body:
      return Body;
    case Usage::BodyStrong:
      return BodyStrong;
    case Usage::BodyLarge:
      return BodyLarge;
    case Usage::Subtitle:
      return Subtitle;
    case Usage::Title:
      return Title;
    case Usage::TitleLarge:
      return TitleLarge;
    case Usage::Display:
      return Display;
  }
  std::unreachable();
}
}// namespace FredEmmott::GUI::SystemFont