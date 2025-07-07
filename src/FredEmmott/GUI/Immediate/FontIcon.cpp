// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

void FontIcon(std::string_view glyph, FontIconSize size, const ID id) {
  const Style styles {
    .mAlignSelf = YGAlignCenter,
    .mFont = {ResolveGlyphFont(size), !important},
  };
  Label(glyph, id);
  immediate_detail::GetCurrentNode()->SetAdditionalBuiltInStyles(styles);
}

void FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyphs,
  FontIconSize size,
  const ID id) {
  const auto font = ResolveGlyphFont(size);
  const Style styles {
    .mFont = {font, !important},
    .mPosition = YGPositionTypeRelative,
  };

  bool first = true;
  immediate_detail::BeginWidget<Widgets::Widget>(id);
  std::size_t count = 0;

  float offset {};

  for (auto&& [glyph, style]: glyphs) {
    auto thisStyle = styles + style;
    if (first) {
      offset = -font.MeasureTextWidth(glyph);
      first = false;
    } else {
      thisStyle.mLeft = offset;
    }

    Label(glyph, ID {count++});
    immediate_detail::GetCurrentNode()->SetAdditionalBuiltInStyles(thisStyle);
  }

  immediate_detail::EndWidget<Widgets::Widget>();
}

}// namespace FredEmmott::GUI::Immediate