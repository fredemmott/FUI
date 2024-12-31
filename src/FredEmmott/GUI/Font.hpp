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
  Font() : Font(SkFont {}) {
  }

  Font(SystemFont::Usage usage) : Font(Resolve(usage)) {
  }
  Font(WidgetFont::Usage usage) : Font(Resolve(usage)) {
  }

  Font(const SkFont& f);

  SkScalar GetMetricsInPixels(SkFontMetrics* metrics) const;
  SkScalar GetSpacingInPixels() const;

  operator SkFont() const noexcept {
    return mFont;
  }

  auto operator->() const noexcept {
    return &mFont;
  }

  bool operator==(const Font& other) const noexcept {
    return mFont == other.mFont;
  }

  SkScalar GetFontSizeInPixels(this const auto& self) noexcept {
    return font_detail::PointsToPixels(self->getSize());
  }

 private:
  struct MetricsInPixels {
    MetricsInPixels() = delete;
    MetricsInPixels(const SkFont&);

    SkScalar mSize {};
    SkScalar mLineSpacing {};
    SkFontMetrics mMetrics {};
  };

  SkFont mFont;
  MetricsInPixels mMetricsInPixels;
};

}// namespace FredEmmott::GUI