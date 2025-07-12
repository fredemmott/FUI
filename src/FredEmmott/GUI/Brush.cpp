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

}// namespace FredEmmott::GUI
