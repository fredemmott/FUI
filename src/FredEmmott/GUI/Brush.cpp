// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Brush.hpp"

namespace FredEmmott::GUI {

// Not in header to avoid inlining in order to reduce binary size, especially
// for the StaticTheme files
Brush::~Brush() = default;
Brush::Brush(const Brush&) = default;
Brush::Brush(Brush&& other) noexcept = default;
Brush& Brush::operator=(const Brush&) = default;
Brush& Brush::operator=(Brush&& other) noexcept = default;

std::optional<Color> Brush::GetSolidColor() const {
  using T = std::optional<Color>;
  return std::visit(
    felly::overload {
      [](const SolidColorBrush& brush) -> T { return brush; },
      [](const AcrylicBrush& brush) -> T { return brush.Resolve(); },
      [](const LinearGradientBrush&) -> T { return std::nullopt; }},
    mBrush);
}

}// namespace FredEmmott::GUI
