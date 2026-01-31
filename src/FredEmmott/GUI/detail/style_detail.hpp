// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/yoga.h>

#include <type_traits>

#include "FredEmmott/GUI/StylePropertyTypes.hpp"

namespace FredEmmott::GUI {
template <class T>
class StyleProperty;
}

#define FUI_ENUM_STYLE_PROPERTY_TYPES(X) \
  X(Brush, Brush) \
  X(GUI::Cursor, Cursor) \
  X(GUI::Font, Font) \
  X(float, Float) \
  X(YGAlign, YGAlign) \
  X(YGBoxSizing, YGBoxSizing) \
  X(YGDisplay, YGDisplay) \
  X(YGFlexDirection, YGFlexDirection) \
  X(YGJustify, YGJustify) \
  X(YGOverflow, YGOverflow) \
  X(YGPositionType, YGPositionType) \
  X(YGWrap, YGWrap) \
  X(GUI::PointerEvents, PointerEvents) \
  X(GUI::TextAlign, TextAlign)

#define FUI_ENUM_STYLE_PROPERTIES(X) \
  X(AlignContent, YGAlign, Self) \
  X(AlignItems, YGAlign, Self) \
  X(AlignSelf, YGAlign, Self) \
  X(BackgroundColor, Brush, Self) \
  X(BorderBottomLeftRadius, float, Self) \
  X(BorderBottomRightRadius, float, Self) \
  X(BorderBottomWidth, float, Self) \
  X(BorderColor, Brush, Self) \
  X(BorderLeftWidth, float, Self) \
  X(BorderRadius, float, Self) \
  X(BorderRightWidth, float, Self) \
  X(BorderTopLeftRadius, float, Self) \
  X(BorderTopRightRadius, float, Self) \
  X(BorderTopWidth, float, Self) \
  X(BorderWidth, float, Self) \
  X(BoxSizing, YGBoxSizing, Self) \
  X(Bottom, float, Self) \
  X(Color, Brush, SelfAndDescendants) \
  X(Cursor, GUI::Cursor, SelfAndDescendants) \
  X(Display, YGDisplay, Self) \
  X(FlexBasis, float, Self) \
  X(FlexDirection, YGFlexDirection, Self) \
  X(FlexGrow, float, Self) \
  X(FlexShrink, float, Self) \
  X(Font, GUI::Font, SelfAndDescendants) \
  X(Gap, float, Self) \
  X(Height, float, Self) \
  X(JustifyContent, YGJustify, Self) \
  X(Left, float, Self) \
  X(Margin, float, Self) \
  X(MarginBottom, float, Self) \
  X(MarginLeft, float, Self) \
  X(MarginRight, float, Self) \
  X(MarginTop, float, Self) \
  X(MaxHeight, float, Self) \
  X(MaxWidth, float, Self) \
  X(MinHeight, float, Self) \
  X(MinWidth, float, Self) \
  X(Opacity, float, Self) \
  X(OutlineColor, Brush, Self) \
  X(OutlineRadius, float, Self) \
  X(OutlineOffset, float, Self) \
  X(OutlineBottomOffset, float, Self) \
  X(OutlineLeftOffset, float, Self) \
  X(OutlineRightOffset, float, Self) \
  X(OutlineTopOffset, float, Self) \
  X(OutlineWidth, float, Self) \
  X(Overflow, YGOverflow, Self) \
  X(Padding, float, Self) \
  X(PaddingBottom, float, Self) \
  X(PaddingLeft, float, Self) \
  X(PaddingRight, float, Self) \
  X(PaddingTop, float, Self) \
  X(PointerEvents, GUI::PointerEvents, Self) \
  X(Position, YGPositionType, Self) \
  X(Right, float, Self) \
  X(ScaleX, float, Self) \
  X(ScaleY, float, Self) \
  X(TextAlign, GUI::TextAlign, SelfAndDescendants) \
  X(Top, float, Self) \
  X(TransformOriginX, float, Self) \
  X(TransformOriginY, float, Self) \
  X(TranslateX, float, Self) \
  X(TranslateY, float, Self) \
  X(Width, float, Self)

/** Properties of the form `mPrefixSuffix`, `mPrefixEdgeSuffix`
 *
 * e.g. `mMarginLeft`, `mBorderLeftWidth`
 */
#define FUI_STYLE_EDGE_PROPERTIES(X) \
  X(Margin, ) \
  X(Padding, ) \
  X(Border, Width) \
  X(Outline, Offset)

namespace FredEmmott::GUI::style_detail {

template <class T>
struct default_t {
  static constexpr auto value = std::nullopt;
};

template <>
struct default_t<Cursor> {
  static constexpr auto value = Cursor::Default;
};

template <>
struct default_t<float> {
  static constexpr float value {YGUndefined};
};

template <class T>
  requires std::is_scoped_enum_v<T>
struct default_t<T> {
  static constexpr T value {};
};

template <>
struct default_t<YGOverflow> {
  static constexpr YGOverflow value {YGOverflowVisible};
};

template <class T>
constexpr auto default_v = default_t<T>::value;

enum class StylePropertyKey {
#define FUI_DECLARE_STYLE_PROPERTY(NAME, ...) NAME,
  FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_STYLE_PROPERTY)
#undef FUI_DECLARE_STYLE_PROPERTY
};

consteval auto last_value_v(auto only) {
  return only;
}
consteval auto last_value_v(auto, auto second, auto... rest) {
  return last_value_v(second, rest...);
}

template <StylePropertyKey P>
struct property_metadata_t;

#define FUI_DECLARE_DEFAULT_PROPERTY_VALUE(NAME, TYPE, SCOPE, ...) \
  template <> \
  struct property_metadata_t<StylePropertyKey::NAME> { \
    using value_type = TYPE; \
    static constexpr auto default_value { \
      last_value_v(default_t<TYPE>::value, ##__VA_ARGS__)}; \
    static constexpr auto default_scope = StylePropertyScope::SCOPE; \
  };
FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_DEFAULT_PROPERTY_VALUE)
#undef FUI_DECLARE_STYLE_PROPERTY
template <StylePropertyKey P>
static constexpr auto default_property_value_v
  = property_metadata_t<P>::default_value;
template <StylePropertyKey P>
static constexpr auto default_property_scope_v
  = property_metadata_t<P>::default_scope;
template <StylePropertyKey P>
using property_value_t = typename property_metadata_t<P>::value_type;

constexpr StylePropertyScope GetDefaultPropertyScope(
  const StylePropertyKey prop) {
  switch (prop) {
#define FUI_DECLARE_PROPERTY_CASE(NAME, TYPE, SCOPE) \
  case StylePropertyKey::NAME: \
    return StylePropertyScope::SCOPE;
    FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_PROPERTY_CASE)
#undef FUI_DECLARE_PROPERTY_CASE
  }
  std::unreachable();
}

template <class... Value>
void VisitStyleProperty(
  const StylePropertyKey key,
  auto&& visitor,
  Value&&... value) {
  switch (key) {
#define PROPERTY_CASE(NAME, TYPE, SCOPE) \
  case StylePropertyKey::NAME: \
    std::invoke( \
      visitor, get<StyleProperty<TYPE>>(std::forward<Value>(value))...); \
    return;
    FUI_ENUM_STYLE_PROPERTIES(PROPERTY_CASE)
#undef PROPERTY_CASE
  }
  throw std::logic_error("Invalid property key");
}

}// namespace FredEmmott::GUI::style_detail
