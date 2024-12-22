// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "SystemFont.hpp"
#include <skia/core/SkFontMetrics.h>
#include <optional>

namespace FredEmmott::GUI {

class Font {
 public:
  Font() = delete;

  Font(SystemFont::Usage usage) : Font(Resolve(usage)) {
  }

  Font(const SkFont& f);

  SkScalar GetHeightInPixels(this const auto& self) noexcept {
    return (self->getSize() * USER_DEFAULT_SCREEN_DPI) / 72;
  }

  SkScalar GetMetricsInPixels(SkFontMetrics* metrics) const;

  operator SkFont() const noexcept {
    return mFont;
  }

  auto operator->() const noexcept {
    return &mFont;
  }

 private:
  struct MetricsInPixels {
    MetricsInPixels() = delete;
    MetricsInPixels(const SkFont&);

    SkScalar mLineSpacing {};
    SkFontMetrics mMetrics {};
  };

  SkFont mFont;
  MetricsInPixels mMetricsInPixels;
};

}// namespace FredEmmott::GUI