// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

Result<> FontIcon(std::string_view glyph, FontIconSize size, const ID id) {
  const auto ret = Label(glyph, id);
  immediate_detail::GetCurrentNode()->BuiltInStyles()
    += Style()
         .AlignSelf(YGAlignCenter)
         .Font(ResolveGlyphFont(size), !important);
  return {ret};
}

Result<> FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyphs,
  FontIconSize size,
  const ID id) {
  const auto font = ResolveGlyphFont(size);
  const auto styles
    = Style().Font(font, !important).Position(YGPositionTypeRelative);

  bool first = true;
  const auto ret = immediate_detail::BeginWidget<Widgets::Widget>(id);
  std::size_t count = 0;

  float offset {};

  for (auto&& [glyph, style]: glyphs) {
    auto thisStyle = styles + style;
    if (first) {
      offset = -font.MeasureTextWidth(glyph);
      first = false;
    } else {
      thisStyle.Left() = offset;
    }

    Label(glyph, ID {count++});
    immediate_detail::GetCurrentNode()->BuiltInStyles() += thisStyle;
  }

  immediate_detail::EndWidget<Widgets::Widget>();
  return {ret};
}

}// namespace FredEmmott::GUI::Immediate