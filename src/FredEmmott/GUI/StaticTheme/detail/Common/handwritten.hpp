// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <chrono>

namespace FredEmmott::GUI::StaticTheme::Common::inline Animations {

// Copied from the XML, but there's so few of them it seemed better
// to hardcode them here than further complicate the XAML parser/codegen
constexpr std::array<float, 4> ControlFastOutSlowInKeySpline {0, 0, 0, 1};
constexpr std::chrono::milliseconds ControlNormalAnimationDuration {250};
constexpr std::chrono::milliseconds ControlFastAnimationDuration {167};
constexpr std::chrono::milliseconds ControlFastAnimationAfterDuration {168};
constexpr std::chrono::milliseconds ControlFasterAnimationDuration {83};

}// namespace FredEmmott::GUI::StaticTheme::Common::inline Animations