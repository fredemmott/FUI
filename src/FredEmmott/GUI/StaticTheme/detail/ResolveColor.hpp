// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/StaticTheme/Theme.hpp>
#include <FredEmmott/GUI/Brush.hpp>

#include "StaticThemedLinearGradientBrush.hpp"

namespace FredEmmott::GUI::StaticTheme {

template<Theme TTheme>
auto ResolveBrush(auto brush) {
  return brush;
}
template<Theme TTheme>
auto ResolveBrush(const StaticThemedLinearGradientBrush& brush) {
  return brush.Resolve<TTheme>();
}
}