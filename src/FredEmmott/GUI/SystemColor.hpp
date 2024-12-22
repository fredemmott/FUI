// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <unordered_map>

namespace FredEmmott::GUI {

class SystemColor final {
 public:
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

  [[nodiscard]] static const SkColor& Get(Usage usage) noexcept {
    return Get().mColors.at(usage);
  }

  auto Background() const noexcept {
    return mColors.at(Usage::Background);
  }

  auto Foreground() const noexcept {
    return mColors.at(Usage::Foreground);
  }

  auto AccentDark3() const noexcept {
    return mColors.at(Usage::AccentDark3);
  }

  auto AccentDark2() const noexcept {
    return mColors.at(Usage::AccentDark3);
  }

  auto AccentDark1() const noexcept {
    return mColors.at(Usage::AccentDark3);
  }

  auto Accent() const noexcept {
    return mColors.at(Usage::Accent);
  }

  auto AccentLight1() const noexcept {
    return mColors.at(Usage::AccentLight1);
  }

  auto AccentLight2() const noexcept {
    return mColors.at(Usage::AccentLight2);
  }

  auto AccentLight3() const noexcept {
    return mColors.at(Usage::AccentLight3);
  }

  static const SystemColor& Get();
 private:
  SystemColor();
  std::unordered_map<Usage, SkColor> mColors;

  void Populate();
};

}// namespace FredEmmott::GUI