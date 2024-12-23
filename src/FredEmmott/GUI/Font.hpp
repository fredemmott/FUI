// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkFont.h>
#include <skia/core/SkFontMetrics.h>

#include <FredEmmott/GUI/detail/font_detail.hpp>

#include "SystemFont.hpp"
#include "WidgetFont.hpp"

namespace FredEmmott::GUI {

class Font {
 public:
  Font() = delete;

  Font(SystemFont::Usage usage) : Font(Resolve(usage)) {
  }
  Font(WidgetFont::Usage usage) : Font(Resolve(usage)) {
  }

  Font(const SkFont& f);

  SkScalar GetHeightInPixels(this const auto& self) noexcept {
    return (self->getSize() * font_detail::BaselineDPI) / 72;
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