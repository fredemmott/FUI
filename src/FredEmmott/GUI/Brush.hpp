// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>
#include <skia/core/SkRect.h>

#include <variant>

#include "Color.hpp"
#include "LinearGradientBrush.hpp"
#include "SolidColorBrush.hpp"
#include "StaticTheme.hpp"

namespace FredEmmott::GUI {

class Brush final {
 public:
  Brush() = delete;
  constexpr Brush(const Brush&) = default;
  constexpr Brush(const std::convertible_to<Color> auto& color)
    : mBrush(SolidColorBrush {Color {color}}) {
  }

  constexpr Brush(const LinearGradientBrush& brush) : mBrush(brush) {
  }

  Brush(StaticTheme::BrushType key) : mBrush(key) {
  }

  [[nodiscard]] SkPaint GetPaint(const SkRect& rect) const {
    if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
      SkPaint paint;
      paint.setColor(*it);
      return paint;
    }
    if (const auto it = get_if<LinearGradientBrush>(&mBrush)) {
      return it->GetPaint(rect);
    }
    if (const auto it = get_if<StaticTheme::BrushType>(&mBrush)) {
      return StaticTheme::Resolve(*it).GetPaint(rect);
    }
    std::unreachable();
  }

  /** If this is a SolidColorBrush, returns the backing color.
   */
  [[nodiscard]] std::optional<Color> GetSolidColor() const {
    if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
      return *it;
    }
    if (const auto it = get_if<StaticTheme::BrushType>(&mBrush)) {
      return StaticTheme::Resolve(*it).GetSolidColor();
    }
    return std::nullopt;
  }

  constexpr bool operator==(const Brush&) const noexcept = default;

 private:
  // Probably change to SolidColorBrush, unique_ptr<BaseBrush> if we end up
  // wanting more than just LinearGradientBrush, but it's worth special-casing
  // SolidColorBrush
  std::variant<SolidColorBrush, LinearGradientBrush, StaticTheme::BrushType>
    mBrush;
};

}// namespace FredEmmott::GUI