// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <FredEmmott/GUI/StaticTheme/Common/detail/macros.hpp>
#include <array>
#include <chrono>

#include "StaticTheme/Resource.hpp"

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

/** Match the current Windows (System) them.
 *
 * The implementation will call `SystemTheme::Refresh()` for you.
 */
void Refresh();

// Generated from the XML files in WinUI3
#define DECLARE_FUI_STATIC_THEME_BRUSH(X) extern const Resource<Brush>* X;
FUI_WINUI_THEME_BRUSHES(DECLARE_FUI_STATIC_THEME_BRUSH)
#undef DECLARE_FUI_STATIC_THEME_BRUSH
#define DECLARE_FUI_STATIC_THEME_COLOR(X) extern const Resource<Color>* X;
FUI_WINUI_THEME_COLORS(DECLARE_FUI_STATIC_THEME_COLOR)
#undef DECLARE_FUI_STATIC_THEME_BRUSH

// Also copied from the XML, but there's so few of them it seemed better
// to hardcode them here than further complicate the XAML parser/codegen
constexpr std::array<float, 4> ControlFastOutSlowInKeySpline {0, 0, 0, 1};
constexpr std::chrono::milliseconds ControlNormalAnimationDuration {250};
constexpr std::chrono::milliseconds ControlFastAnimationDuration {167};
constexpr std::chrono::milliseconds ControlFastAnimationAfterDuration {168};
constexpr std::chrono::milliseconds ControlFasterAnimationDuration {83};
};// namespace FredEmmott::GUI::StaticTheme