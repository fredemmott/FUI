// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "WidgetColor.hpp"

#include "Color.hpp"
#include "SystemColor.hpp"

namespace FredEmmott::GUI::WidgetColor {

Color Resolve(const Usage usage) noexcept {
  switch (usage) {
    case ControlElevationBorder:
      return Color { SystemColor::Foreground } * (0x08 / 255.0);
    case CardBackgroundFillDefault:
      return Color { SystemColor::Foreground } * (0x27 / 255.0);
    case ControlFillDefault:
      return Color { SystemColor::Foreground } * (0x16 / 255.0);
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::WidgetColor