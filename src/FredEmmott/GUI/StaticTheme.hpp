// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/WinUI3Themes/Enums.hpp>
#include <array>
#include <chrono>

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

// Also copied from the XML, but there's so few of them it semmed better
// to hardcode them here than further complicate the XAML parser/codegen
constexpr std::array<float, 4> ControlFastOutSlowInKeySpline {0, 0, 0, 1};
constexpr std::chrono::milliseconds ControlNormalAnimationDuration {250};
constexpr std::chrono::milliseconds ControlFastAnimationDuration {167};
constexpr std::chrono::milliseconds ControlFastAnimationAfterDuration {168};
constexpr std::chrono::milliseconds ControlFasterAnimationDuration {83};

};// namespace FredEmmott::GUI::StaticTheme