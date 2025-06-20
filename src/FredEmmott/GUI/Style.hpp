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

struct Style {
  using Selector = std::variant<StyleClass, const Widgets::Widget*>;

  StyleProperty<YGAlign> mAlignItems;
  StyleProperty<YGAlign> mAlignSelf;
  StyleProperty<Brush> mBackgroundColor;
  StyleProperty<Brush> mBorderColor;
  StyleProperty<SkScalar> mBorderRadius;
  StyleProperty<SkScalar> mBorderWidth;
  StyleProperty<SkScalar> mBottom;
  InheritableStyleProperty<Brush> mColor;
  StyleProperty<YGDisplay, YGDisplayFlex> mDisplay;
  StyleProperty<SkScalar> mFlexBasis;
  StyleProperty<YGFlexDirection> mFlexDirection;
  StyleProperty<SkScalar, 0.0f> mFlexGrow;
  StyleProperty<SkScalar, 0.0f> mFlexShrink;
  InheritableStyleProperty<Font> mFont;
  StyleProperty<SkScalar> mGap;
  StyleProperty<SkScalar> mHeight;
  StyleProperty<SkScalar> mLeft;
  StyleProperty<SkScalar> mMargin;
  StyleProperty<SkScalar> mMarginBottom;
  StyleProperty<SkScalar> mMarginLeft;
  StyleProperty<SkScalar> mMarginRight;
  StyleProperty<SkScalar> mMarginTop;
  StyleProperty<SkScalar> mMaxHeight;
  StyleProperty<SkScalar> mMaxWidth;
  StyleProperty<SkScalar> mMinHeight;
  StyleProperty<SkScalar> mMinWidth;
  StyleProperty<SkScalar, 1.0f> mOpacity;
  StyleProperty<YGOverflow> mOverflow;
  StyleProperty<SkScalar> mPadding;
  StyleProperty<SkScalar> mPaddingBottom;
  StyleProperty<SkScalar> mPaddingLeft;
  StyleProperty<SkScalar> mPaddingRight;
  StyleProperty<SkScalar> mPaddingTop;
  StyleProperty<YGPositionType, YGPositionTypeRelative> mPosition;
  StyleProperty<SkScalar> mRight;
  StyleProperty<SkScalar, 1.0f> mScaleX;
  StyleProperty<SkScalar, 1.0f> mScaleY;
  StyleProperty<SkScalar> mTop;
  StyleProperty<SkScalar, 0.0f> mTranslateX;
  StyleProperty<SkScalar, 0.0f> mTranslateY;
  StyleProperty<SkScalar> mWidth;

  std::vector<std::tuple<Selector, Style>> mAnd;

  [[nodiscard]] Style InheritableValues() const noexcept;
  [[nodiscard]]
  static Style BuiltinBaseline();

  Style& operator+=(const Style& other) noexcept;
  Style operator+(const Style& other) const noexcept {
    Style ret {*this};
    ret += other;
    return ret;
  }

  bool operator==(const Style& other) const noexcept = default;
};

}// namespace FredEmmott::GUI

#define FUI_STYLE_PROPERTIES(X) \
  X(AlignItems) \
  X(AlignSelf) \
  X(BackgroundColor) \
  X(BorderColor) \
  X(BorderRadius) \
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
  X(Position) \
  X(Right) \
  X(ScaleX) \
  X(ScaleY) \
  X(Top) \
  X(TranslateX) \
  X(TranslateY) \
  X(Width)