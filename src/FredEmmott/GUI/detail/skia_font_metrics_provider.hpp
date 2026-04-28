// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Shared implementation of FontMetricsProvider for the Skia-backed window
// backends (Win32 D3D12 + Skia, SDL3 + Skia/Vulkan). Separate header lets
// each backend's window TU SetFontMetricsProvider one of these without
// duplicating the body — no single build configuration links both backends
// today, but they should agree on metrics regardless.
#pragma once

#include <skia/core/SkFont.h>
#include <skia/core/SkFontMetrics.h>

#include <FredEmmott/GUI/Font.hpp>
#include <FredEmmott/GUI/detail/renderer_detail.hpp>
#include <FredEmmott/GUI/detail/skia_paragraph.hpp>

#include <limits>
#include <string_view>

namespace FredEmmott::GUI::renderer_detail {

class SkiaFontMetricsProvider final : public FontMetricsProvider {
 public:
  ~SkiaFontMetricsProvider() override = default;

  float MeasureTextWidth(const Font& font, const std::string_view text)
    const override {
    if (!font) {
      return std::numeric_limits<float>::quiet_NaN();
    }
    // SkParagraph for per-codepoint typeface fallback (issue #76) so this
    // matches what SkiaRenderer::DrawText actually paints when the typeface
    // lacks glyphs (e.g. emoji served by Noto Color Emoji).
    auto paragraph = skia_paragraph_detail::BuildSingleStyleParagraph(
      font.as<SkFont>(), text);
    paragraph->layout(std::numeric_limits<float>::infinity());
    return paragraph->getMaxIntrinsicWidth();
  }

  Font::Metrics GetFontMetrics(const Font& font) const override {
    // Empty Font (variant holds std::monostate) is reachable on Linux when
    // the requested system font isn't installed (e.g. "Segoe Fluent Icons"
    // for FontIcons and fontconfig has no substitute). font.as<SkFont>()
    // does an unchecked std::get and would throw bad_variant_access; mirror
    // MeasureTextWidth's guard and return zeroed metrics instead.
    if (!font) {
      return {};
    }
    const auto sk = font.as<SkFont>();
    SkFontMetrics pt {};
    const auto lineSpacing = sk.getMetrics(&pt);
    return {
      .mSize = sk.getSize(),
      .mLineSpacing = lineSpacing,
      .mAscent = pt.fAscent,
      .mDescent = pt.fDescent,
    };
  }
};

}// namespace FredEmmott::GUI::renderer_detail
