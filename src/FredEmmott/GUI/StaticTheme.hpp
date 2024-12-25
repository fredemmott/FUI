// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/WinUI3Themes/Enums.hpp>

namespace FredEmmott::GUI {
class Brush;
class Color;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::StaticTheme {

using BrushType = gui_detail::WinUI3Themes::Brushes;
using ColorType = gui_detail::WinUI3Themes::Colors;
using enum BrushType;
using enum ColorType;
Brush Resolve(BrushType brush) noexcept;
Color Resolve(ColorType color) noexcept;
};// namespace FredEmmott::GUI::StaticTheme