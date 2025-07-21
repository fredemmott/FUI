// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Generic.handwritten.hpp"

#include <FredEmmott/GUI/Font.hpp>

namespace FredEmmott::GUI::StaticTheme::Generic {

// These are mostly taken from TextBlock_themeresources.xaml instead of
// Generic_themeresources.xaml, but there's some overlap. Putting them here
// as we don't use TextBlock_themeresources.xaml for anything else

// This seems like a pointless helper, but it cuts a fair bit off the binary
// size
static Style GetFontStyle(const SystemFont::Usage usage) {
  return Style().Font(usage);
}

const ImmutableStyle& BaseTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::BodyStrong)};
  return ret;
}
const ImmutableStyle& CaptionTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::Caption)};
  return ret;
}
const ImmutableStyle& BodyTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::Body)};
  return ret;
}
const ImmutableStyle& SubtitleTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::Subtitle)};
  return ret;
}
const ImmutableStyle& TitleTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::Title)};
  return ret;
}
const ImmutableStyle& TitleLargeTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::TitleLarge)};
  return ret;
}
const ImmutableStyle& DisplayTextBlockStyle() {
  static const ImmutableStyle ret {GetFontStyle(SystemFont::Display)};
  return ret;
}

const ImmutableStyle& TextBlockClassStyles() {
  static const ImmutableStyle ret {
    Style(BodyTextBlockStyle())
      .And(CaptionTextBlockClass, CaptionTextBlockStyle())
      .And(BodyStrongTextBlockClass, BodyStrongTextBlockStyle())
      .And(SubtitleTextBlockClass, SubtitleTextBlockStyle())
      .And(TitleTextBlockClass, TitleTextBlockStyle())
      .And(TitleLargeTextBlockClass, TitleLargeTextBlockStyle())
      .And(DisplayTextBlockClass, DisplayTextBlockStyle()),
  };
  return ret;
}

}// namespace FredEmmott::GUI::StaticTheme::Generic
