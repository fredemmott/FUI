// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <FredEmmott/GUI/detail/WinUI3Themes/Enums.hpp>
#include <variant>

#include "SystemTheme.hpp"

namespace FredEmmott::GUI {
class Color;
}
namespace FredEmmott::GUI::StaticTheme {

using ColorType = gui_detail::WinUI3Themes::Colors;
Color Resolve(ColorType) noexcept;
}// namespace FredEmmott::GUI::StaticTheme

namespace FredEmmott::GUI {

class Color final {
 public:
  Color() = delete;
  constexpr Color(SkColor color) : mVariant(color) {
  }
  constexpr Color(StaticTheme::ColorType u) : mVariant(u) {
  }
  constexpr Color(SystemTheme::ColorType u) : mVariant(u) {
  }

  Color operator*(std::floating_point auto alpha) const noexcept {
    const auto self = this->Resolve();
    return SkColorSetA(self, alpha * SkColorGetA(self));
  }

  constexpr operator SkColor() const noexcept {
    return this->Resolve();
  }

  constexpr bool operator==(const Color& other) const noexcept = default;

 private:
  std::variant<SkColor, StaticTheme::ColorType, SystemTheme::ColorType>
    mVariant;

  constexpr SkColor Resolve() const noexcept {
    if (std::holds_alternative<SkColor>(mVariant)) {
      return std::get<SkColor>(mVariant);
    }
    if (std::holds_alternative<StaticTheme::ColorType>(mVariant)) {
      return StaticTheme::Resolve(std::get<StaticTheme::ColorType>(mVariant));
    }
    if (std::holds_alternative<SystemTheme::ColorType>(mVariant)) {
      return SystemTheme::Resolve(std::get<SystemTheme::ColorType>(mVariant));
    }
    std::unreachable();
  }
};

}// namespace FredEmmott::GUI