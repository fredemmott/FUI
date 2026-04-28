// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Per-codepoint font fallback for SkFont-based text rendering.
//
// SkCanvas::drawString and SkFont::measureText both operate on a single
// SkTypeface and have no automatic fallback for codepoints that typeface
// doesn't carry — they substitute glyph 0 (.notdef / "tofu") instead.
// Win32 sidesteps this with DirectWrite which is fallback-aware. On Linux
// we drive Skia directly, so emoji and similar out-of-typeface codepoints
// would render as boxes regardless of which fonts are installed.
//
// ForEachRun walks UTF-8 text and splits it into runs of consecutive
// codepoints covered by the same SkTypeface, looking up fallbacks via
// SkFontMgr::matchFamilyStyleCharacter (which queries fontconfig on Linux).
// Callers draw or measure each run with the run's font and concatenate the
// results — same input text, fallback-aware behaviour.
//
// The proper long-term answer is SkParagraph (already wired into FUI for
// TextBlock); this lighter helper covers the imperative DrawText path
// without that bigger refactor.
#pragma once

#include <skia/core/SkFont.h>

#include <functional>
#include <string_view>

class SkFontMgr;
class SkTypeface;

namespace FredEmmott::GUI::skia_text_fallback_detail {

// One run from ForEachRun: a slice of the original byte sequence and the
// SkFont (cloned with the appropriate typeface) to render it with.
struct Run {
  SkFont mFont;
  std::string_view mText;
};

// True if `tf` carries any of the OpenType color-glyph tables (COLR/CBDT/
// sbix). Used to reject monochrome typefaces that fontconfig may pick first
// for emoji codepoints (e.g. Segoe UI Symbol covers U+1F600 with a
// black-and-white outline; Noto Color Emoji is what we actually want).
bool HasColorGlyphTables(SkTypeface* tf) noexcept;

// Splits `text` into runs by typeface availability and invokes `cb(Run)`
// for each, in order. `fontMgr` may be null — if it is, every codepoint
// stays in `mainFont` (no fallback) and `cb` is called once.
//
// Implementation uses ICU's U8_NEXT_OR_FFFD for UTF-8 iteration and
// UCHAR_EMOJI_PRESENTATION for the color-emoji escalation check, so this
// header-pollution stays light (one std::function indirection per call,
// negligible vs. Skia draw cost).
void ForEachRun(
  const SkFont& mainFont,
  std::string_view text,
  SkFontMgr* fontMgr,
  const std::function<void(const Run&)>& cb);

}// namespace FredEmmott::GUI::skia_text_fallback_detail
