// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <stdexcept>
#include <variant>

#include "Color.hpp"
#include "LinearGradientBrush.hpp"
#include "Rect.hpp"
#include "SolidColorBrush.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkPaint.h>
#endif

#ifdef FUI_ENABLE_DIRECT2D
#include <d2d1.h>
#include <wil/com.h>
#endif

namespace FredEmmott::GUI {
class Brush final {
 public:
  Brush() = delete;
  constexpr Brush(const std::convertible_to<Color> auto& color)
    : mBrush(SolidColorBrush {Color {color}}) {}

  Brush(const LinearGradientBrush& brush) : mBrush(brush) {}

  /** If this is a SolidColorBrush, returns the backing color.
   */
  [[nodiscard]] std::optional<Color> GetSolidColor() const {
    if (const auto it = get_if<SolidColorBrush>(&mBrush)) {
      return *it;
    }
    return std::nullopt;
  }

  constexpr bool operator==(const Brush&) const noexcept = default;

#ifdef FUI_ENABLE_SKIA
  [[nodiscard]] SkPaint GetSkiaPaint(const SkRect& rect) const;
#endif
#ifdef FUI_ENABLE_DIRECT2D
  [[nodiscard]] wil::com_ptr<ID2D1Brush> GetDirect2DBrush(
    ID2D1RenderTarget* rt,
    const Rect& rect) const;
#endif
 private:
  // Probably change to SolidColorBrush, unique_ptr<BaseBrush> if we end up
  // wanting more than just LinearGradientBrush, but it's worth special-casing
  // SolidColorBrush
  std::variant<SolidColorBrush, LinearGradientBrush> mBrush;
#ifdef FUI_ENABLE_DIRECT2D
  // TODO: make SolidColorBrush a class and put this there
  mutable wil::com_ptr<ID2D1SolidColorBrush> mD2DSolidColorBrush;
#endif
};
}// namespace FredEmmott::GUI