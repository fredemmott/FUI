// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/WinUI3Themes/Enums.hpp>

namespace FredEmmott::GUI {
class Brush;
class Color;
}// namespace FredEmmott::GUI

/** A 'Static Theme' is compiled-in, based on the XAML files taken from
 * WinUI3.
 *
 * 'Static' is used in the web sense of 'static resources', matching WinUI3's
 * terminology
 *
 * The active Static Theme is selected automatically based on the current
 * System theme (using light vs dark heuristics), and may change at runtime.
 */
namespace FredEmmott::GUI::StaticTheme {

using BrushType = gui_detail::WinUI3Themes::Brushes;
using ColorType = gui_detail::WinUI3Themes::Colors;

using enum BrushType;
// not using the `ColorType` enum as brushes are generally preferable

Brush Resolve(BrushType brush) noexcept;
Color Resolve(ColorType color) noexcept;

enum class ThemeKind {
  Light,
  Dark,
  HighContrast,
};
ThemeKind GetCurrentThemeKind();

/** Match the current Windows (System) them.
 *
 * The implementation will call `SystemTheme::Refresh()` for you.
 */
void Refresh();

};// namespace FredEmmott::GUI::StaticTheme