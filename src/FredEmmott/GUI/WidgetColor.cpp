// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "WidgetColor.hpp"

#include "Color.hpp"
#include "SystemColor.hpp"

namespace FredEmmott::GUI::WidgetColor {

Color Resolve(Usage usage) noexcept {
  constexpr auto background = Color {SystemColor::Background};
  switch (usage) {
    case CardBackgroundFillDefault:
      return background.MixIn(0.167, SystemColor::Foreground);
    case ControlFillDefault:
      return background.MixIn(0.22, SystemColor::Foreground);
  }
  std::unreachable();
}

}// namespace FredEmmott::GUI::WidgetColor