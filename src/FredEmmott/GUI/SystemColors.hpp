// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <unordered_map>

namespace FredEmmott::GUI {

class SystemColors final {
 public:
  SystemColors();

  auto Background() const noexcept {
    return Get(Usage::Background);
  }

  auto Foreground() const noexcept {
    return Get(Usage::Foreground);
  }

  auto AccentDark3() const noexcept {
    return Get(Usage::AccentDark3);
  }

  auto AccentDark2() const noexcept {
    return Get(Usage::AccentDark3);
  }

  auto AccentDark1() const noexcept {
    return Get(Usage::AccentDark3);
  }

  auto Accent() const noexcept {
    return Get(Usage::Accent);
  }

  auto AccentLight1() const noexcept {
    return Get(Usage::AccentLight1);
  }

  auto AccentLight2() const noexcept {
    return Get(Usage::AccentLight2);
  }

  auto AccentLight3() const noexcept {
    return Get(Usage::AccentLight3);
  }

 private:
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
  std::unordered_map<Usage, SkColor> mColors;

  void Populate();
};

}// namespace FredEmmott::GUI