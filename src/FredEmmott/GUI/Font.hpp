// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/font_detail.hpp>
#include <optional>
#include <string_view>
#include <variant>

#include "SystemFont.hpp"
#include "WidgetFont.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFont.h>
#include <skia/core/SkFontMetrics.h>
#endif

namespace FredEmmott::GUI {

class Font {
 public:
  struct Metrics {
    float mSize {};
    float mLineSpacing {};
    float mDescent {};
    constexpr bool operator==(const Metrics&) const noexcept = default;
  };
  Font() {}
  Font(SystemFont::Usage usage) : Font(Resolve(usage)) {}
  Font(WidgetFont::Usage usage) : Font(Resolve(usage)) {}

  [[nodiscard]]
  const Metrics& GetMetrics() const noexcept {
    return mMetrics;
  }

  [[nodiscard]]
  Font WithSize(float pixels) const noexcept;

  [[nodiscard]]
  float MeasureTextWidth(std::string_view) const noexcept;

#ifdef FUI_ENABLE_SKIA
  Font(const SkFont& f);
  operator SkFont() const noexcept;
#endif

  constexpr bool operator==(const Font& other) const noexcept {
    return mFont == other.mFont;
  }

 private:
  Metrics mMetrics;
  std::variant<
#ifdef FUI_ENABLE_SKIA
    SkFont,
#endif
    std::monostate>
    mFont {std::monostate {}};
};

}// namespace FredEmmott::GUI