// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Brush.hpp"

namespace FredEmmott::GUI {

// Not in header to avoid inlining in order to reduce binary size, especially
// for the StaticTheme files
Brush::~Brush() = default;

}// namespace FredEmmott::GUI
