// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>

#include "SystemColor.hpp"

namespace FredEmmott::GUI {

class Color {
 public:
  Color() = delete;
  explicit constexpr Color(SystemColor::Usage u) : mVariant(u) {
  }
  explicit constexpr Color(SkColor color) : mVariant(color) {
  }

  constexpr operator const SkColor&() const noexcept {
    return this->Get();
  }

  constexpr auto operator->() const noexcept {
    return &this->Get();
  }
 private:
  std::variant<SystemColor::Usage, SkColor> mVariant;

  constexpr const SkColor& Get() const noexcept {
    if (std::holds_alternative<SkColor>(mVariant)) {
      return std::get<SkColor>(mVariant);
    }
    return SystemColor::Get(std::get<SystemColor::Usage>(mVariant));
  }
};

}// namespace FredEmmott::GUI