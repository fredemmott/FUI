// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "FredEmmott/GUI/StaticTheme/ScrollBar.hpp"

namespace FredEmmott::GUI::Widgets::ScrollBarDetail {
using namespace StaticTheme::ScrollBar;

constexpr auto ContractAnimation = CubicBezierStyleTransition(
  ScrollBarContractBeginTime,
  ScrollBarOpacityChangeDuration,
  StaticTheme::Common::ControlFastOutSlowInKeySpline);
constexpr auto ExpandAnimation = CubicBezierStyleTransition(
  ScrollBarExpandBeginTime,
  ScrollBarOpacityChangeDuration,
  StaticTheme::Common::ControlFastOutSlowInKeySpline);
}// namespace FredEmmott::GUI::Widgets::ScrollBarDetail