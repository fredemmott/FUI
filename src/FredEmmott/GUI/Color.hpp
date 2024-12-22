// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkColor.h>

#include <variant>

#include "SystemColor.hpp"
#include "WidgetColor.hpp"

namespace FredEmmott::GUI {

class Color {
 public:
  Color() = delete;
  constexpr Color(SkColor color) : mVariant(color) {
  }
  constexpr Color(WidgetColor::Usage u) : mVariant(u) {
  }
  constexpr Color(SystemColor::Usage u) : mVariant(u) {
  }

  constexpr operator SkColor() const noexcept {
    return this->Resolve();
  }

  Color MixIn(SkScalar ratio, const Color&) const noexcept;

 private:
  std::variant<SkColor, WidgetColor::Usage, SystemColor::Usage> mVariant;

  constexpr SkColor Resolve() const noexcept {
    if (std::holds_alternative<SkColor>(mVariant)) {
      return std::get<SkColor>(mVariant);
    }
    if (std::holds_alternative<WidgetColor::Usage>(mVariant)) {
      return WidgetColor::Resolve(std::get<WidgetColor::Usage>(mVariant));
    }
    if (std::holds_alternative<SystemColor::Usage>(mVariant)) {
      return SystemColor::Resolve(std::get<SystemColor::Usage>(mVariant));
    }
    std::unreachable();
  }
};

}// namespace FredEmmott::GUI