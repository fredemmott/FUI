// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/style/lazy_init_style.hpp>

namespace FredEmmott::GUI::StaticTheme::Generic {

// These are mostly taken from TextBlock_themeresources.xaml instead of
// Generic_themeresources.xaml, but there's some overlap. Putting them here
// as we don't use TextBlock_themeresources.xaml for anything else

extern const style_detail::lazy_init_style BaseTextBlockStyle;

extern const style_detail::lazy_init_style CaptionTextBlockStyle;
extern const style_detail::lazy_init_style BodyTextBlockStyle;
inline const auto& BodyStrongTextBlockStyle = BaseTextBlockStyle;
extern const style_detail::lazy_init_style SubtitleTextBlockStyle;
extern const style_detail::lazy_init_style TitleTextBlockStyle;
extern const style_detail::lazy_init_style TitleLargeTextBlockStyle;
extern const style_detail::lazy_init_style DisplayTextBlockStyle;

}// namespace FredEmmott::GUI::StaticTheme::Generic