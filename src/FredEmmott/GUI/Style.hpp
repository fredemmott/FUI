// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>

#include "Color.hpp"
#include "Font.hpp"

namespace FredEmmott::GUI {

struct Style {
  template <class T>
  struct InheritableValue : std::optional<T> {};
  template <class T>
  using Value = std::optional<T>;

  Value<Color> mBackgroundColor;
  Value<Color> mBorderColor;
  Value<SkScalar> mBorderRadius;
  InheritableValue<Color> mColor;
  InheritableValue<Font> mFont;
  Value<SkScalar> mMargin;
  Value<SkScalar> mPadding;

  Style& operator+=(const Style& other) noexcept;
  Style operator+(const Style& other) const noexcept {
    Style ret {*this};
    ret += other;
    return ret;
  }
};

}// namespace FredEmmott::GUI