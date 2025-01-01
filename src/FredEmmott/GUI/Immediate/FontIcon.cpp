// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

void FontIcon(std::string_view glyph, FontIconSize size, const ID id) {
  using Widgets::WidgetStyles;

  const WidgetStyles styles {
    .mBase = {
      .mFont = ResolveGlyphFont(size),
    }};
  Label(glyph, id);
  immediate_detail::GetCurrentNode()->SetExplicitStyles(styles);
}

void FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyphs,
  FontIconSize size,
  const ID id) {
  using Widgets::WidgetStyles;

  const WidgetStyles styles {
    .mBase = {
      .mFont = ResolveGlyphFont(size),
    }};

  bool first = true;
  immediate_detail::BeginWidget<Widgets::Widget>(id);
  std::size_t count = 0;
  for (auto&& [glyph, style]: glyphs) {
    auto thisStyle = styles + style;
    if (first) {
      first = false;
    } else {
      thisStyle.mBase.mPosition = YGPositionTypeAbsolute;
    }

    Label(glyph, ID {count++});
    immediate_detail::GetCurrentNode()->SetExplicitStyles(thisStyle);
  }

  immediate_detail::EndWidget<Widgets::Widget>();
}

}// namespace FredEmmott::GUI::Immediate