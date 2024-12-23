// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>

#include "Color.hpp"
#include "Font.hpp"

namespace FredEmmott::GUI {

struct Style {
  template <class T>
  struct InheritableValue : std::optional<T> {
    constexpr bool operator==(const InheritableValue& other) const noexcept
      = default;
  };
  template <class T>
  using Value = std::optional<T>;

  Value<Color> mBackgroundColor;
  Value<Color> mBorderColor;
  Value<SkScalar> mBorderRadius;
  Value<SkScalar> mBorderWidth;
  InheritableValue<Color> mColor;
  InheritableValue<Font> mFont;
  Value<SkScalar> mHeight;
  Value<SkScalar> mMargin;
  Value<SkScalar> mPadding;
  Value<SkScalar> mPaddingBottom;
  Value<SkScalar> mPaddingLeft;
  Value<SkScalar> mPaddingRight;
  Value<SkScalar> mPaddingTop;
  Value<SkScalar> mWidth;

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