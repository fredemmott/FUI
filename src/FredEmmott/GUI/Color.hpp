// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>

#include "SystemColor.hpp"

namespace FredEmmott::GUI {

struct Color : private std::variant<SystemColor::Usage, SkColor> {
  Color() = delete;
  explicit Color(SystemColor::Usage u) : variant(u) {
  }
  explicit Color(SkColor color) : variant(color) {
  }

  const SkColor& Get(this const auto& self) noexcept {
    if (std::holds_alternative<SkColor>(self)) {
      return std::get<SkColor>(self);
    }
    return SystemColor::Get(std::get<SystemColor::Usage>(self));
  }

  operator const SkColor&() const noexcept {
    return this->Get();
  }

  auto operator->() const noexcept {
    return &this->Get();
  }
};

}// namespace FredEmmott::GUI