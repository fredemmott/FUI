// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <unordered_map>

namespace FredEmmott::GUI {

class Colors final {
public:
  Colors();

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
  SkColor Get(Usage) const noexcept;
private:
  std::unordered_map<Usage, SkColor> mColors;

  void Populate();
};

}