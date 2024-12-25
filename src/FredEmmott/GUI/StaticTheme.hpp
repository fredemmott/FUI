// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/WinUI3Themes/Enums.hpp>

namespace FredEmmott::GUI {
class Brush;
class Color;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::StaticTheme {

using gui_detail::WinUI3Themes::Brushes;
using gui_detail::WinUI3Themes::Colors;
using enum Brushes;
using enum Colors;
Brush Resolve(Brushes brush) noexcept;
Color Resolve(Colors color) noexcept;
};// namespace FredEmmott::GUI::StaticTheme