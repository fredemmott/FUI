// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Single-style, unwrapped SkParagraph for the imperative DrawText /
// MeasureTextWidth paths. Gives them SkParagraph's per-codepoint typeface
// fallback (issue #76 fix) without per-call boilerplate. Caller must
// layout(width) the returned Paragraph before paint / measurement.
#pragma once

#include <skia/core/SkFont.h>
#include <skia/modules/skparagraph/include/Paragraph.h>

#include <memory>
#include <string_view>

namespace FredEmmott::GUI::skia_paragraph_detail {

std::unique_ptr<skia::textlayout::Paragraph> BuildSingleStyleParagraph(
  const SkFont& font,
  std::string_view text);

}// namespace FredEmmott::GUI::skia_paragraph_detail
