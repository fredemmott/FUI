// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>

#include <unordered_set>

#include "Brush.hpp"
#include "Font.hpp"
#include "PseudoClasses.hpp"
#include "StyleClass.hpp"
#include "StyleProperty.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI {

enum class TextAlign {
  Left,
  Center,
  Right,
};

enum class PointerEvents {
  Auto,
  None,
};

struct Style {
  using Selector
    = std::variant<std::monostate, StyleClass, const Widgets::Widget*>;

  StyleProperty<YGAlign> mAlignContent;
  StyleProperty<YGAlign> mAlignItems;
  StyleProperty<YGAlign> mAlignSelf;
  StyleProperty<Brush> mBackgroundColor;
  StyleProperty<float> mBorderBottomLeftRadius;
  StyleProperty<float> mBorderBottomRightRadius;
  StyleProperty<float> mBorderBottomWidth;
  StyleProperty<Brush> mBorderColor;
  StyleProperty<float> mBorderLeftWidth;
  StyleProperty<float> mBorderRadius;
  StyleProperty<float> mBorderRightWidth;
  StyleProperty<float> mBorderTopLeftRadius;
  StyleProperty<float> mBorderTopRightRadius;
  StyleProperty<float> mBorderTopWidth;
  StyleProperty<float> mBorderWidth;
  StyleProperty<float> mBottom;
  InheritableStyleProperty<Brush> mColor;
  StyleProperty<YGDisplay, YGDisplayFlex> mDisplay;
  StyleProperty<float> mFlexBasis;
  StyleProperty<YGFlexDirection> mFlexDirection;
  StyleProperty<float, 0.0f> mFlexGrow;
  StyleProperty<float, 0.0f> mFlexShrink;
  InheritableStyleProperty<Font> mFont;
  StyleProperty<float> mGap;
  StyleProperty<float> mHeight;
  StyleProperty<YGJustify> mJustifyContent;
  StyleProperty<float> mLeft;
  StyleProperty<float> mMargin;
  StyleProperty<float> mMarginBottom;
  StyleProperty<float> mMarginLeft;
  StyleProperty<float> mMarginRight;
  StyleProperty<float> mMarginTop;
  StyleProperty<float> mMaxHeight;
  StyleProperty<float> mMaxWidth;
  StyleProperty<float> mMinHeight;
  StyleProperty<float> mMinWidth;
  StyleProperty<float, 1.0f> mOpacity;
  StyleProperty<YGOverflow> mOverflow;
  StyleProperty<float> mPadding;
  StyleProperty<float> mPaddingBottom;
  StyleProperty<float> mPaddingLeft;
  StyleProperty<float> mPaddingRight;
  StyleProperty<float> mPaddingTop;
  StyleProperty<PointerEvents> mPointerEvents;
  StyleProperty<YGPositionType, YGPositionTypeRelative> mPosition;
  StyleProperty<float> mRight;
  StyleProperty<float, 1.0f> mScaleX;
  StyleProperty<float, 1.0f> mScaleY;
  StyleProperty<float> mTop;
  InheritableStyleProperty<TextAlign, TextAlign::Left> mTextAlign;
  StyleProperty<float, 0.0f> mTranslateX;
  StyleProperty<float, 0.0f> mTranslateY;
  StyleProperty<float> mWidth;

  std::vector<std::tuple<Selector, Style>> mAnd;
  std::vector<std::tuple<Selector, Style>> mDescendants;

  [[nodiscard]] Style InheritableValues() const noexcept;
  [[nodiscard]]
  static Style BuiltinBaseline();

  Style& operator+=(const Style& other) noexcept;

  bool operator==(const Style& other) const noexcept = default;
};

inline Style operator+(const Style& lhs, const Style& rhs) noexcept {
  Style ret {lhs};
  ret += rhs;
  return ret;
}

}// namespace FredEmmott::GUI

#define FUI_STYLE_PROPERTIES(X) \
  X(AlignContent) \
  X(AlignItems) \
  X(AlignSelf) \
  X(BackgroundColor) \
  X(BorderBottomWidth) \
  X(BorderBottomLeftRadius) \
  X(BorderBottomRightRadius) \
  X(BorderColor) \
  X(BorderLeftWidth) \
  X(BorderRadius) \
  X(BorderRightWidth) \
  X(BorderTopLeftRadius) \
  X(BorderTopRightRadius) \
  X(BorderTopWidth) \
  X(BorderWidth) \
  X(Bottom) \
  X(Color) \
  X(Display) \
  X(Font) \
  X(FlexBasis) \
  X(FlexDirection) \
  X(FlexShrink) \
  X(FlexGrow) \
  X(Gap) \
  X(Height) \
  X(JustifyContent) \
  X(Left) \
  X(Margin) \
  X(MarginBottom) \
  X(MarginLeft) \
  X(MarginRight) \
  X(MarginTop) \
  X(MaxHeight) \
  X(MaxWidth) \
  X(MinHeight) \
  X(MinWidth) \
  X(Opacity) \
  X(Overflow) \
  X(Padding) \
  X(PaddingBottom) \
  X(PaddingLeft) \
  X(PaddingRight) \
  X(PaddingTop) \
  X(PointerEvents) \
  X(Position) \
  X(Right) \
  X(ScaleX) \
  X(ScaleY) \
  X(TextAlign) \
  X(Top) \
  X(TranslateX) \
  X(TranslateY) \
  X(Width)