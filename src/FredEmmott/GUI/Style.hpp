// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>

#include "Color.hpp"
#include "Font.hpp"

namespace FredEmmott::GUI {

struct Style {
  std::optional<Color> mColor;
  std::optional<Color> mBackgroundColor;
  std::optional<Color> mBorderColor;

  std::optional<Font> mFont;

  Style& operator+=(const Style& other) noexcept;
  Style operator+(const Style& other) const noexcept {
    Style ret {*this};
    ret += other;
    return ret;
  }
};

}// namespace FredEmmott::GUI