// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>

#include "SystemColor.hpp"

namespace FredEmmott::GUI {

class Color {
 public:
  Color() = delete;
  constexpr Color(SystemColor::Usage u) : mVariant(u) {
  }
  constexpr Color(SkColor color) : mVariant(color) {
  }

  constexpr operator SkColor() const noexcept {
    return this->Get();
  }

  Color MixIn(SkScalar ratio, const Color&) const noexcept;

 private:
  std::variant<SystemColor::Usage, SkColor> mVariant;

  constexpr SkColor Get() const noexcept {
    if (std::holds_alternative<SkColor>(mVariant)) {
      return std::get<SkColor>(mVariant);
    }
    return SystemColor::Resolve(std::get<SystemColor::Usage>(mVariant));
  }
};

}// namespace FredEmmott::GUI