// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/yoga.h>

#include <type_traits>

namespace FredEmmott::GUI::style_detail {

template <class T>
struct default_t {
  static constexpr auto value = std::nullopt;
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

}// namespace FredEmmott::GUI::style_detail

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
  X(Bottom, float, Self) \
  X(Color, Brush, SelfAndDescendants) \
  X(Display, YGDisplay, Self, YGDisplayFlex) \
  X(FlexBasis, float, Self) \
  X(FlexDirection, YGFlexDirection, Self) \
  X(FlexGrow, float, Self, 0.0f) \
  X(FlexShrink, float, Self, 0.0f) \
  X(Font, Font, SelfAndDescendants) \
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
  X(Opacity, float, Self, 1.0f) \
  X(Overflow, YGOverflow, Self) \
  X(Padding, float, Self) \
  X(PaddingBottom, float, Self) \
  X(PaddingLeft, float, Self) \
  X(PaddingRight, float, Self) \
  X(PaddingTop, float, Self) \
  X(PointerEvents, PointerEvents, Self) \
  X(Position, YGPositionType, Self, YGPositionTypeRelative) \
  X(Right, float, Self) \
  X(ScaleX, float, Self, 1.0f) \
  X(ScaleY, float, Self, 1.0f) \
  X(TextAlign, TextAlign, SelfAndDescendants, TextAlign::Left) \
  X(Top, float, Self) \
  X(TranslateX, float, Self, 0.0f) \
  X(TranslateY, float, Self, 0.0f) \
  X(Width, float, Self)

/** Properties of the form `mPrefixSuffix`, `mPrefixEdgeSuffix`
 *
 * e.g. `mMarginLeft`, `mBorderLeftWidth`
 */
#define FUI_STYLE_EDGE_PROPERTIES(X) \
  X(Margin, ) \
  X(Padding, ) \
  X(Border, Width)
