// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Generic.handwritten.hpp"

#include <FredEmmott/GUI/Font.hpp>

namespace FredEmmott::GUI::StaticTheme::Generic {

// These are mostly taken from TextBlock_themeresources.xaml instead of
// Generic_themeresources.xaml, but there's some overlap. Putting them here
// as we don't use TextBlock_themeresources.xaml for anything else

const style_detail::lazy_init_style BaseTextBlockStyle {[] {
  return Style {
    .mFont = {SystemFont::BodyStrong},
  };
}};
const style_detail::lazy_init_style CaptionTextBlockStyle {[] {
  return BaseTextBlockStyle
    + Style {
      .mFont = {SystemFont::Caption},
    };
}};
const style_detail::lazy_init_style BodyTextBlockStyle {[] {
  return BaseTextBlockStyle
    + Style {
      .mFont = {SystemFont::Body},
    };
}};
const style_detail::lazy_init_style SubtitleTextBlockStyle {[] {
  return BaseTextBlockStyle
    + Style {
      .mFont = {SystemFont::Subtitle},
    };
}};
const style_detail::lazy_init_style TitleTextBlockStyle {[] {
  return BaseTextBlockStyle
    + Style {
      .mFont = {SystemFont::Title},
    };
}};
const style_detail::lazy_init_style TitleLargeTextBlockStyle {[] {
  return BaseTextBlockStyle
    + Style {
      .mFont = {SystemFont::TitleLarge},
    };
}};
const style_detail::lazy_init_style DisplayTextBlockStyle {[] {
  return BaseTextBlockStyle
    + Style {
      .mFont = {SystemFont::Display},
    };
}};

}// namespace FredEmmott::GUI::StaticTheme::Generic