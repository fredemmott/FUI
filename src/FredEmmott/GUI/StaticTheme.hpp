// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/StaticTheme/Common.hpp>

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

using namespace StaticTheme::Common;

/** Match the current Windows (System) them.
 *
 * The implementation will call `SystemTheme::Refresh()` for you.
 */
void Refresh();
};// namespace FredEmmott::GUI::StaticTheme