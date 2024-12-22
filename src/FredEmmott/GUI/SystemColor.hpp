// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

namespace FredEmmott::GUI {
class Color;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::SystemColor {

// This library is a visual clone of WinUI3, so even if we're ported to
// another platform, we're going to base the color usages off of the
// Windows theme colors (Windows::UI::ViewManagement::UIColorType)
enum class Usage {
  Background,
  Foreground,
  AccentDark3,
  AccentDark2,
  AccentDark1,
  Accent,
  AccentLight1,
  AccentLight2,
  AccentLight3,
};

Color Get(Usage usage) noexcept;

Color Background() noexcept;
Color Foreground() noexcept;
Color AccentDark3() noexcept;
Color AccentDark2() noexcept;
Color AccentDark1() noexcept;
Color Accent() noexcept;
Color AccentLight1() noexcept;
Color AccentLight2() noexcept;
Color AccentLight3() noexcept;

}// namespace FredEmmott::GUI::SystemColor