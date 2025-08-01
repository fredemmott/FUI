// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FontIcon.hpp"

#include "FredEmmott/GUI/StaticTheme/Generic.hpp"
#include "FredEmmott/GUI/Widgets/Label.hpp"
#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

using namespace FredEmmott::GUI::StaticTheme::Generic;

namespace FredEmmott::GUI::Immediate {

namespace {

constexpr LiteralStyleClass StackedStyleClass {"FontIcon/Stacked"};

Style MakeFontIconStyle(const FontIconSize size) {
  const auto font = ResolveGlyphFont(size);
  const auto width = font.GetMetrics().mSize;
  FUI_ASSERT(width == font.MeasureTextWidth("\ue700"));
  return Style().Font(font).Width(width).Height(width).And(
    StackedStyleClass, Style().Left(-width));
}

const ImmutableStyle& FontIconStyle() {
  static ImmutableStyle ret {
    Style()
      .AlignSelf(YGAlignCenter)
      .Position(YGPositionTypeRelative)
      .And(CaptionTextBlockClass, MakeFontIconStyle(FontIconSize::Caption))
      .And(BodyTextBlockClass, MakeFontIconStyle(FontIconSize::Body))
      .And(
        BodyStrongTextBlockClass, MakeFontIconStyle(FontIconSize::BodyStrong))
      .And(SubtitleTextBlockClass, MakeFontIconStyle(FontIconSize::Subtitle))
      .And(TitleTextBlockClass, MakeFontIconStyle(FontIconSize::Title))
      .And(
        TitleLargeTextBlockClass, MakeFontIconStyle(FontIconSize::TitleLarge))
      .And(DisplayTextBlockClass, MakeFontIconStyle(FontIconSize::Display)),
  };
  return ret;
}

StyleClass GetStyleClass(const FontIconSize size) {
  using enum FontIconSize;
  switch (size) {
    case Caption:
      return CaptionTextBlockClass;
    case Body:
      return BodyTextBlockClass;
    case BodyStrong:
      return BodyStrongTextBlockClass;
    case Subtitle:
      return SubtitleTextBlockClass;
    case Title:
      return TitleTextBlockClass;
    case TitleLarge:
      return TitleLargeTextBlockClass;
    case Display:
      return DisplayTextBlockClass;
    default:
      std::unreachable();
  }
}
}// namespace

Result<>
FontIcon(const std::string_view glyph, const FontIconSize size, const ID id) {
  const auto label = immediate_detail::BeginWidget<Widgets::Label>(
    id, FontIconStyle(), StyleClasses {GetStyleClass(size)});
  label->SetText(glyph);
  immediate_detail::EndWidget<Widgets::Label>();
  return {label};
}

Result<> FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyphs,
  FontIconSize size,
  const ID id) {
  const auto ret
    = immediate_detail::BeginWidget<Widgets::Widget>(id, ImmutableStyle {});
  std::size_t count = 0;

  bool first = true;
  for (auto&& [glyph, style]: glyphs) {
    FontIcon(glyph, size, ID {count++});
    const auto widget = immediate_detail::GetCurrentNode();
    if (!std::exchange(first, false)) {
      widget->AddStyleClass(StackedStyleClass);
    }
    widget->SetMutableStyles(style);
  }

  immediate_detail::EndWidget<Widgets::Widget>();
  return {ret};
}

}// namespace FredEmmott::GUI::Immediate