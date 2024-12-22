// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>

#include "SystemFont.hpp"

namespace FredEmmott::GUI {

struct Font : private std::variant<SystemFont::Usage, SkFont> {
  Font() = delete;
  explicit Font(SystemFont::Usage usage) : variant(usage) {
  }
  explicit Font(const SkFont& f) : variant(f) {
  }

  const SkFont& Get(this const auto& self) noexcept {
    if (std::holds_alternative<SkFont>(self)) {
      return std::get<SkFont>(self);
    }
    return SystemFont::Get(std::get<SystemFont::Usage>(self));
  }

  SkScalar GetPixelHeight(this const auto& self) noexcept {
    return (self->getSize() * USER_DEFAULT_SCREEN_DPI) / 72;
  }

  operator const SkFont&() const noexcept {
    return this->Get();
  }

  auto operator->() const noexcept {
    return &this->Get();
  }
};

}// namespace FredEmmott::GUI