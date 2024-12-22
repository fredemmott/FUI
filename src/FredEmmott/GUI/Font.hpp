// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <variant>

#include "SystemFont.hpp"

namespace FredEmmott::GUI {

class Font {
public:
  Font() = delete;
  constexpr Font(SystemFont::Usage usage) : mVariant(usage) {
  }
  constexpr Font(const SkFont& f) : mVariant(f) {
  }

  SkScalar GetPixelHeight(this const auto& self) noexcept {
    return (self->getSize() * USER_DEFAULT_SCREEN_DPI) / 72;
  }

  operator SkFont() const noexcept {
    return this->Get();
  }

  auto operator->() const noexcept {
    struct wrapper_t {
      SkFont mFont;
      auto operator->() const noexcept {
        return &mFont;
      }
    };
    return wrapper_t { this->Get() };
  }

private:
  std::variant<SystemFont::Usage, SkFont> mVariant;

  SkFont Get() const noexcept {
    if (std::holds_alternative<SkFont>(mVariant)) {
      return std::get<SkFont>(mVariant);
    }
    return SystemFont::Get(std::get<SystemFont::Usage>(mVariant));
  }
};

}// namespace FredEmmott::GUI