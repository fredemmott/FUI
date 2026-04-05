// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <stdexcept>
#include <variant>

#include "AcrylicBrush.hpp"
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
class Renderer;

namespace detail {
template <class T>
struct is_native_brush_t : std::false_type {};

#ifdef FUI_ENABLE_SKIA
template <>
struct is_native_brush_t<SkPaint> : std::true_type {};
#endif

#ifdef FUI_ENABLE_DIRECT2D
template <>
struct is_native_brush_t<ID2D1Brush*> : std::true_type {};
#endif
}// namespace detail

template <class T>
concept native_brush = detail::is_native_brush_t<T>::value;

class Brush final {
 public:
  Brush() = delete;
  ~Brush();
  constexpr Brush(const std::convertible_to<Color> auto& color)
    : mBrush(SolidColorBrush {Color {color}}) {}

  Brush(const AcrylicBrush& brush) : mBrush(brush) {}
  Brush(const LinearGradientBrush& brush) : mBrush(brush) {}
  Brush(const Brush&);
  Brush(Brush&&) noexcept;
  Brush& operator=(const Brush&);
  Brush& operator=(Brush&&) noexcept;

  /** If this is a SolidColorBrush, returns the backing color.
   */
  [[nodiscard]] std::optional<Color> GetSolidColor() const;

  constexpr bool operator==(const Brush&) const noexcept = default;

  template <native_brush T>
  T as(Renderer*, const Rect&) const;

 private:
  // Probably change to SolidColorBrush, unique_ptr<BaseBrush> if we end up
  // wanting more than just LinearGradientBrush, but it's worth special-casing
  // SolidColorBrush
  std::variant<SolidColorBrush, LinearGradientBrush, AcrylicBrush> mBrush;
};
}// namespace FredEmmott::GUI