// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

void FontIcon(
  const Widgets::WidgetStyles& explicitStyles,
  std::string_view glyph,
  FontIconSize size) {
  using Widgets::WidgetStyles;

  const auto styles = WidgetStyles {
    .mBase = {
      .mFont = ResolveGlyphFont(size),
    }
  } + explicitStyles;
  Label(styles, "{}", glyph);
}

void FontIcon(
  const Widgets::WidgetStyles& explicitStyles,
  std::initializer_list<StackedFontIconGlyph> glyphs,
  FontIconSize size) {
  using Widgets::WidgetStyles;

  const auto styles = WidgetStyles {
    .mBase = {
      .mFont = ResolveGlyphFont(size),
    }
  } + explicitStyles;

  auto subStyles = styles.InheritableStyles()
    + WidgetStyles {
      .mBase = {
        .mLeft = 0,
      },
    };

  bool first = true;
  immediate_detail::BeginWidget<Widgets::Widget> {}();
  for (auto&& [glyph, style]: glyphs) {
    auto thisStyle = style;
    if (first) {
      first = false;
    } else {
      thisStyle += WidgetStyles {{.mPosition = YGPositionTypeAbsolute}};
    }

    Label(styles + thisStyle, "{}", glyph);
  }

  immediate_detail::EndWidget<Widgets::Widget>();
}

}// namespace FredEmmott::GUI::Immediate