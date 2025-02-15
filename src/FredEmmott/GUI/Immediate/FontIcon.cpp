// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {
const auto FontIconStyleClass = StyleClass::Make("FontIcon");
}

void FontIcon(std::string_view glyph, FontIconSize size, const ID id) {
  static const StyleSheet styles {{
    FontIconStyleClass,
    Style {
      .mFont = {ResolveGlyphFont(size), !important},
    },
  }};
  Label(glyph, id);

  auto label = immediate_detail::GetCurrentNode();
  label->AppendBuiltInStyleSheet(styles);
  label->AddStyleClass(FontIconStyleClass);
}

void FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyphs,
  FontIconSize size,
  const ID id) {
  bool first = true;
  immediate_detail::BeginWidget<Widgets::Widget>(id);
  std::size_t count = 0;
  for (auto&& [glyph, style]: glyphs) {
    FontIcon(glyph, size, ID {count++});
    auto node = immediate_detail::GetCurrentNode();
    if (first) {
      first = false;
    } else {
      node->AppendBuiltInStyleSheet({{
        StyleSelector {node},
        Style {.mPosition = YGPositionTypeAbsolute},
      }});
    }
    node->SetExplicitStyles(style);
  }

  immediate_detail::EndWidget<Widgets::Widget>();
}

}// namespace FredEmmott::GUI::Immediate