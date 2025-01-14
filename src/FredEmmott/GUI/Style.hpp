// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>

#include <unordered_set>

#include "Brush.hpp"
#include "Font.hpp"
#include "StyleProperty.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI {

struct Style {
  class Class {
    friend class std::hash<Class>;

   public:
    Class() = delete;
    Class(const Class&) = default;
    Class(Class&&) = default;
    Class& operator=(const Class&) = default;
    Class& operator=(Class&&) = default;

    static Class Make(std::string_view name);

    bool operator==(const Class&) const noexcept = default;

   private:
    Class(std::size_t id) : mID(id) {}
    std::size_t mID {};
  };
  enum class PseudoClass {
    Active,
    Disabled,
    Hover,
  };
  using Selector = std::variant<PseudoClass, Class, const Widgets::Widget*>;
  using ClassList = std::unordered_set<Class>;

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
  StyleProperty<SkScalar> mMinHeight;
  StyleProperty<SkScalar> mMinWidth;
  StyleProperty<SkScalar, 1.0f> mOpacity;
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
  std::vector<std::tuple<Selector, Style>> mDescendants;

  [[nodiscard]] Style InheritableValues() const noexcept;

  Style& operator+=(const Style& other) noexcept;
  Style operator+(const Style& other) const noexcept {
    Style ret {*this};
    ret += other;
    return ret;
  }

  bool operator==(const Style& other) const noexcept = default;
};

Style::ClassList operator+(const Style::ClassList& lhs, Style::Class rhs);

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
  X(MinHeight) \
  X(MinWidth) \
  X(Opacity) \
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

template <>
struct std::hash<FredEmmott::GUI::Style::Class> {
  std::size_t operator()(
    const FredEmmott::GUI::Style::Class& c) const noexcept {
    return std::hash<std::size_t> {}(c.mID);
  }
};