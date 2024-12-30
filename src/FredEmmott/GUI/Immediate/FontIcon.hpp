// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/Widgets/WidgetStyles.hpp>

namespace FredEmmott::GUI::Immediate {

using FontIconSize = SystemFont::Usage;

void FontIcon(
  const Widgets::WidgetStyles&,
  std::string_view glyph,
  FontIconSize size = FontIconSize::Body);

inline void FontIcon(
  std::string_view glyph,
  FontIconSize size = FontIconSize::Body) {
  FontIcon({}, glyph, size);
}

struct StackedFontIconGlyph {
  std::string_view mGlyph;
  Widgets::WidgetStyles mBase;
};

void FontIcon(
  const Widgets::WidgetStyles&,
  std::initializer_list<StackedFontIconGlyph> glyph,
  FontIconSize size = FontIconSize::Body);

inline void FontIcon(
  std::initializer_list<StackedFontIconGlyph> glyph,
  FontIconSize size = FontIconSize::Body) {
  FontIcon({}, glyph, size);
}

}// namespace FredEmmott::GUI::Immediate