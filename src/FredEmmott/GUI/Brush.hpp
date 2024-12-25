// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>
#include <skia/core/SkRect.h>

#include <variant>

#include "Color.hpp"
#include "LinearGradientBrush.hpp"
#include "SolidColorBrush.hpp"

namespace FredEmmott::GUI {

class Brush final {
 public:
  Brush() = delete;
  constexpr Brush(const std::convertible_to<Color> auto& color)
    : mBrush(SolidColorBrush {Color {color}}) {
  }

  constexpr Brush(const LinearGradientBrush& brush) : mBrush(brush) {
  }

  [[nodiscard]] SkPaint GetPaint(const SkRect&) const {
    SkPaint paint;
    paint.setColor(std::get<SolidColorBrush>(mBrush));
    return paint;
  }

  constexpr bool operator==(const Brush&) const noexcept = default;

 private:
  // Probably change to SolidColorBrush, unique_ptr<BaseBrush> if we end up
  // wanting more than just LinearGradientBrush, but it's worth special-casing
  // SolidColorBrush
  std::variant<SolidColorBrush, LinearGradientBrush> mBrush;
};

}// namespace FredEmmott::GUI