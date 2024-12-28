// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPaint.h>

#include <stdexcept>
#include <variant>

#include "Color.hpp"
#include "LinearGradientBrush.hpp"
#include "SolidColorBrush.hpp"
#include "StaticTheme/Resource.hpp"
#include "StaticTheme/Theme.hpp"

namespace FredEmmott::GUI {

class Brush final {
  using StaticThemeBrush = const StaticTheme::Resource<Brush>*;

 public:
  Brush() = delete;
  constexpr Brush(const Brush&) = default;
  constexpr Brush(const std::convertible_to<Color> auto& color)
    : mBrush(SolidColorBrush {Color {color}}) {
  }

  constexpr Brush(const LinearGradientBrush& brush) : mBrush(brush) {
  }

  Brush(StaticThemeBrush brush) : mBrush(brush) {
    if (!brush) [[unlikely]] {
      throw std::logic_error("Static resource brushes must be a valid pointer");
    }
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
    if (const auto it = get_if<StaticThemeBrush>(&mBrush)) {
      return (*it)->Resolve().GetPaint(rect);
    }
    std::unreachable();
  }

  /** If this is a SolidColorBrush, returns the backing color.
   */
  [[nodiscard]] std::optional<Color> GetSolidColor() const {
    if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
      return *it;
    }
    if (const auto it = get_if<StaticThemeBrush>(&mBrush)) {
      return (*it)->Resolve().GetSolidColor();
    }
    return std::nullopt;
  }

  constexpr bool operator==(const Brush&) const noexcept = default;

 private:
  // Probably change to SolidColorBrush, unique_ptr<BaseBrush> if we end up
  // wanting more than just LinearGradientBrush, but it's worth special-casing
  // SolidColorBrush
  std::variant<SolidColorBrush, LinearGradientBrush, StaticThemeBrush> mBrush;
};

}// namespace FredEmmott::GUI