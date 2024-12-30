// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>

#include "Brush.hpp"
#include "Font.hpp"
#include "StyleProperty.hpp"

namespace FredEmmott::GUI {

struct Style {
  StyleProperty<YGAlign> mAlignSelf;
  StyleProperty<YGAlign> mAlignItems;
  StyleProperty<Brush> mBackgroundColor;
  StyleProperty<Brush> mBorderColor;
  StyleProperty<SkScalar> mBorderRadius;
  StyleProperty<SkScalar> mBorderWidth;
  StyleProperty<SkScalar> mBottom;
  InheritableStyleProperty<Brush> mColor;
  StyleProperty<YGDisplay> mDisplay;
  StyleProperty<YGFlexDirection> mFlexDirection;
  InheritableStyleProperty<Font> mFont;
  StyleProperty<SkScalar> mGap;
  StyleProperty<SkScalar> mHeight;
  StyleProperty<SkScalar> mLeft;
  StyleProperty<SkScalar> mMargin;
  StyleProperty<SkScalar> mMarginBottom;
  StyleProperty<SkScalar> mMarginLeft;
  StyleProperty<SkScalar> mMarginRight;
  StyleProperty<SkScalar> mMarginTop;
  StyleProperty<SkScalar> mPadding;
  StyleProperty<SkScalar> mPaddingBottom;
  StyleProperty<SkScalar> mPaddingLeft;
  StyleProperty<SkScalar> mPaddingRight;
  StyleProperty<SkScalar> mPaddingTop;
  StyleProperty<YGPositionType> mPosition;
  StyleProperty<SkScalar> mRight;
  StyleProperty<SkScalar> mTop;
  StyleProperty<SkScalar> mWidth;

  [[nodiscard]] Style InheritableValues() const noexcept;

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
  X(FlexDirection) \
  X(Gap) \
  X(Height) \
  X(Left) \
  X(Margin) \
  X(MarginBottom) \
  X(MarginLeft) \
  X(MarginRight) \
  X(MarginTop) \
  X(Padding) \
  X(PaddingBottom) \
  X(PaddingLeft) \
  X(PaddingRight) \
  X(PaddingTop) \
  X(Position) \
  X(Right) \
  X(Top) \
  X(Width)