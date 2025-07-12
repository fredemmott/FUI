// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Brush.hpp>
#include <FredEmmott/GUI/StaticTheme/Theme.hpp>

#include "StaticThemedLinearGradientBrush.hpp"

namespace FredEmmott::GUI::StaticTheme {

template <Theme TTheme>
constexpr auto ResolveBrush(auto brush) {
  return brush;
}
template <Theme TTheme>
constexpr auto ResolveBrush(const StaticThemedLinearGradientBrush& brush) {
  return brush.Resolve<TTheme>();
}
}// namespace FredEmmott::GUI::StaticTheme