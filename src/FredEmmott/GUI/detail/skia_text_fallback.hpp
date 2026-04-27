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
#include <skia/core/SkFontMgr.h>
#include <skia/core/SkFourByteTag.h>
#include <skia/core/SkRefCnt.h>
#include <skia/core/SkTypeface.h>
#include <skia/core/SkTypes.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <utility>

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
inline bool HasColorGlyphTables(SkTypeface* const tf) noexcept {
  if (!tf) {
    return false;
  }
  constexpr auto COLR = SkSetFourByteTag('C', 'O', 'L', 'R');
  constexpr auto CBDT = SkSetFourByteTag('C', 'B', 'D', 'T');
  constexpr auto sbix = SkSetFourByteTag('s', 'b', 'i', 'x');
  return tf->getTableSize(COLR) > 0 || tf->getTableSize(CBDT) > 0
    || tf->getTableSize(sbix) > 0;
}

// Pragmatic "this codepoint plausibly wants a color emoji font" test. Not a
// canonical Unicode Emoji-property check — wide enough to catch the common
// pictographs / dingbats / SMP emoji blocks where retrying via the "emoji"
// generic family is worth doing, narrow enough that ordinary symbol or CJK
// codepoints don't get redirected away from a sensible typeface.
inline bool LooksLikeEmojiCodepoint(const SkUnichar cp) noexcept {
  return (cp >= 0x2300 && cp <= 0x27BF) || (cp >= 0x2B00 && cp <= 0x2BFF)
    || (cp >= 0x1F000);
}

// Returns the codepoint at *p* and advances *p* by its UTF-8 byte length.
// Returns U+FFFD on malformed input. *p* is advanced by at least one byte
// even on failure so the loop can't get stuck.
inline SkUnichar DecodeUTF8(const char*& p, const char* const end) noexcept {
  const auto b0 = static_cast<uint8_t>(*p++);
  if (b0 < 0x80) {
    return b0;
  }
  int extra;
  uint32_t cp;
  if ((b0 & 0xE0) == 0xC0) {
    extra = 1;
    cp = b0 & 0x1Fu;
  } else if ((b0 & 0xF0) == 0xE0) {
    extra = 2;
    cp = b0 & 0x0Fu;
  } else if ((b0 & 0xF8) == 0xF0) {
    extra = 3;
    cp = b0 & 0x07u;
  } else {
    return 0xFFFD;
  }
  while (extra-- > 0) {
    if (p >= end) {
      return 0xFFFD;
    }
    const auto b = static_cast<uint8_t>(*p);
    if ((b & 0xC0) != 0x80) {
      return 0xFFFD;
    }
    cp = (cp << 6) | (b & 0x3Fu);
    ++p;
  }
  return static_cast<SkUnichar>(cp);
}

// Splits `text` into runs by typeface availability and invokes `cb(Run)`
// for each. `cb` may be a lambda taking `const Run&`. `fontMgr` may be
// null — if it is, every codepoint stays in `mainFont` (no fallback).
template <class CB>
void ForEachRun(
  const SkFont& mainFont,
  const std::string_view text,
  SkFontMgr* const fontMgr,
  CB&& cb) {
  if (text.empty()) {
    return;
  }
  SkTypeface* const mainTypeface = mainFont.getTypeface();
  if (!mainTypeface || !fontMgr) {
    cb(Run {mainFont, text});
    return;
  }
  const SkFontStyle style = mainTypeface->fontStyle();

  // Cache fallback typefaces per codepoint. Same emoji or symbol typically
  // appears multiple times; we keep an owning sk_sp here so raw pointer
  // comparisons stay valid for the duration of the walk.
  std::unordered_map<SkUnichar, sk_sp<SkTypeface>> fallbacks;

  const auto typefaceFor = [&](const SkUnichar cp) -> SkTypeface* {
    if (mainTypeface->unicharToGlyph(cp) != 0) {
      return mainTypeface;
    }
    const auto it = fallbacks.find(cp);
    if (it != fallbacks.end()) {
      return it->second ? it->second.get() : mainTypeface;
    }
    auto tf = fontMgr->matchFamilyStyleCharacter(
      /*familyName=*/nullptr, style, /*bcp47=*/nullptr, /*bcp47Count=*/0, cp);
    // Fontconfig may pick a monochrome font that happens to cover the
    // codepoint (e.g. Segoe UI Symbol or Noto Emoji) before reaching a
    // color emoji font. If the codepoint plausibly wants color and the
    // first match is monochrome, retry with the "emoji" generic family —
    // fontconfig maps that alias to Noto Color Emoji / Segoe UI Emoji on
    // Linux. Keep the original match if no color font is found.
    if (tf && LooksLikeEmojiCodepoint(cp) && !HasColorGlyphTables(tf.get())) {
      auto colorTf = fontMgr->matchFamilyStyleCharacter(
        "emoji", style, /*bcp47=*/nullptr, /*bcp47Count=*/0, cp);
      if (colorTf && HasColorGlyphTables(colorTf.get())) {
        tf = std::move(colorTf);
      }
    }
    SkTypeface* const raw = tf.get();
    fallbacks.emplace(cp, std::move(tf));
    return raw ? raw : mainTypeface;
  };

  const char* const begin = text.data();
  const char* const end = begin + text.size();
  const char* p = begin;

  size_t runStart = 0;
  SkTypeface* runTypeface = nullptr;

  while (p < end) {
    const char* const cpStart = p;
    const SkUnichar cp = DecodeUTF8(p, end);
    SkTypeface* const tf = typefaceFor(cp);

    if (runTypeface == nullptr) {
      runTypeface = tf;
      runStart = static_cast<size_t>(cpStart - begin);
      continue;
    }
    if (runTypeface == tf) {
      continue;
    }
    // Boundary: flush the current run.
    const auto flushEnd = static_cast<size_t>(cpStart - begin);
    SkFont runFont = mainFont;
    runFont.setTypeface(sk_ref_sp(runTypeface));
    cb(Run {std::move(runFont), text.substr(runStart, flushEnd - runStart)});
    runTypeface = tf;
    runStart = flushEnd;
  }

  if (runTypeface != nullptr) {
    SkFont runFont = mainFont;
    runFont.setTypeface(sk_ref_sp(runTypeface));
    cb(Run {std::move(runFont), text.substr(runStart)});
  }
}

}// namespace FredEmmott::GUI::skia_text_fallback_detail
