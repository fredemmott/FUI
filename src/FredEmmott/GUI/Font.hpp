// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "SystemFont.hpp"

namespace FredEmmott::GUI {

class Font {
 public:
  Font() = delete;

  Font(SystemFont::Usage usage) : mFont(Resolve(usage)) {
  }

  Font(const SkFont& f) : mFont(f) {
  }

  SkScalar GetPixelHeight(this const auto& self) noexcept {
    return (self->getSize() * USER_DEFAULT_SCREEN_DPI) / 72;
  }

  operator SkFont() const noexcept {
    return mFont;
  }

  auto operator->() const noexcept {
    return &mFont;
  }

 private:
  SkFont mFont;
};

}// namespace FredEmmott::GUI