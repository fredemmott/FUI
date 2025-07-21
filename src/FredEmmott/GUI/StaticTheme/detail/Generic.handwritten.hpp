// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::Generic {

// These are mostly taken from TextBlock_themeresources.xaml instead of
// Generic_themeresources.xaml, but there's some overlap. Putting them here
// as we don't use TextBlock_themeresources.xaml for anything else

const ImmutableStyle& BaseTextBlockStyle();
const ImmutableStyle& CaptionTextBlockStyle();
const ImmutableStyle& BodyTextBlockStyle();
inline const auto& BodyStrongTextBlockStyle() {
  return BaseTextBlockStyle();
}
const ImmutableStyle& SubtitleTextBlockStyle();
const ImmutableStyle& TitleTextBlockStyle();
const ImmutableStyle& TitleLargeTextBlockStyle();
const ImmutableStyle& DisplayTextBlockStyle();

constexpr LiteralStyleClass CaptionTextBlockClass {"TextBlock/Caption"};
constexpr LiteralStyleClass BodyTextBlockClass {"TextBlock/Body"};
constexpr LiteralStyleClass BodyStrongTextBlockClass {"TextBlock/BodyStrong"};
constexpr LiteralStyleClass SubtitleTextBlockClass {"TextBlock/Subtitle"};
constexpr LiteralStyleClass TitleTextBlockClass {"TextBlock/Title"};
constexpr LiteralStyleClass TitleLargeTextBlockClass {"TextBlock/TitleLarge"};
constexpr LiteralStyleClass DisplayTextBlockClass {"TextBlock/Display"};

const ImmutableStyle& TextBlockClassStyles();

}// namespace FredEmmott::GUI::StaticTheme::Generic