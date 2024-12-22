// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI {
class Color;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::WidgetColor {
enum class Usage {
  CardBackgroundFillDefault,
  ControlElevationBorder,
  ControlFillDefault,
};
using enum Usage;

Color Resolve(Usage usage) noexcept;
};