// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

void FontIcon(std::string_view glyph, FontIconSize size, const ID id) {
  const Style styles {.mFont = {ResolveGlyphFont(size), !important}};
  Label(glyph, id);
  immediate_detail::GetCurrentNode()->SetAdditionalBuiltInStyles(styles);
}

void FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyphs,
  FontIconSize size,
  const ID id) {
  const Style styles {.mFont = {ResolveGlyphFont(size), !important}};

  bool first = true;
  immediate_detail::BeginWidget<Widgets::Widget>(id);
  std::size_t count = 0;
  for (auto&& [glyph, style]: glyphs) {
    auto thisStyle = styles + style;
    if (first) {
      first = false;
    } else {
      thisStyle.mPosition = YGPositionTypeAbsolute;
    }

    Label(glyph, ID {count++});
    immediate_detail::GetCurrentNode()->SetAdditionalBuiltInStyles(thisStyle);
  }

  immediate_detail::EndWidget<Widgets::Widget>();
}

}// namespace FredEmmott::GUI::Immediate