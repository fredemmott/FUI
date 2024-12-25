// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/WinUI3Themes/Enums.hpp>

namespace FredEmmott::GUI {
class Color;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::WidgetColor {
using Usage = gui_detail::WinUI3Themes::Colors;
using enum Usage;

Color Resolve(Usage usage) noexcept;
};// namespace FredEmmott::GUI::WidgetColor