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

const style_detail::lazy_init_style BaseTextBlockStyle {
  [] { return GetFontStyle(SystemFont::BodyStrong); }};
const style_detail::lazy_init_style CaptionTextBlockStyle {
  [] { return GetFontStyle(SystemFont::Caption); }};
const style_detail::lazy_init_style BodyTextBlockStyle {
  [] { return GetFontStyle(SystemFont::Body); }};
const style_detail::lazy_init_style SubtitleTextBlockStyle {
  [] { return GetFontStyle(SystemFont::Subtitle); }};
const style_detail::lazy_init_style TitleTextBlockStyle {
  [] { return GetFontStyle(SystemFont::Title); }};
const style_detail::lazy_init_style TitleLargeTextBlockStyle {
  [] { return GetFontStyle(SystemFont::TitleLarge); }};
const style_detail::lazy_init_style DisplayTextBlockStyle {
  [] { return GetFontStyle(SystemFont::Display); }};

}// namespace FredEmmott::GUI::StaticTheme::Generic