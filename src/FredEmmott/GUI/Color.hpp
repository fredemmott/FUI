// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <variant>

#include "StaticTheme.hpp"
#include "SystemColor.hpp"

namespace FredEmmott::GUI {

class Color final {
 public:
  Color() = delete;
  constexpr Color(SkColor color) : mVariant(color) {
  }
  constexpr Color(StaticTheme::ColorType u) : mVariant(u) {
  }
  constexpr Color(SystemColor::Usage u) : mVariant(u) {
  }

  Color operator*(std::floating_point auto alpha) const noexcept {
    const auto self = this->Resolve();
    return SkColorSetA(self, alpha * SkColorGetA(self));
  }

  constexpr operator SkColor() const noexcept {
    return this->Resolve();
  }

  Color MixIn(SkScalar ratio, const Color&) const noexcept;

  constexpr bool operator==(const Color& other) const noexcept = default;

 private:
  std::variant<SkColor, StaticTheme::ColorType, SystemColor::Usage> mVariant;

  constexpr SkColor Resolve() const noexcept {
    if (std::holds_alternative<SkColor>(mVariant)) {
      return std::get<SkColor>(mVariant);
    }
    if (std::holds_alternative<StaticTheme::ColorType>(mVariant)) {
      return StaticTheme::Resolve(std::get<StaticTheme::ColorType>(mVariant));
    }
    if (std::holds_alternative<SystemColor::Usage>(mVariant)) {
      return SystemColor::Resolve(std::get<SystemColor::Usage>(mVariant));
    }
    std::unreachable();
  }
};

}// namespace FredEmmott::GUI