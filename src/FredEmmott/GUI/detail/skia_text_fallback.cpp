// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// See skia_text_fallback.hpp for what this is for and why.

#include <FredEmmott/GUI/detail/skia_text_fallback.hpp>

#include <skia/core/SkFontMgr.h>
#include <skia/core/SkFontStyle.h>
#include <skia/core/SkFourByteTag.h>
#include <skia/core/SkRefCnt.h>
#include <skia/core/SkTypeface.h>
#include <skia/core/SkTypes.h>

// FUI already has a hard ICU dependency; ICU's macros are used directly so
// we don't carry a private UTF-8 decoder or a hand-rolled emoji-range
// heuristic. <unicode/utf8.h> provides U8_NEXT_OR_FFFD; <unicode/uchar.h>
// provides u_hasBinaryProperty + the UCHAR_EMOJI_PRESENTATION constant.
// (Skia rendering only happens with FUI_ENABLE_SKIA on, which always means
// vcpkg ICU on the platforms we ship; if a Windows-SDK-ICU configuration
// ever needs this file, route through detail/icu.hpp.)
#include <unicode/uchar.h>
#include <unicode/utf8.h>

#include <cstdint>
#include <unordered_map>
#include <utility>

namespace FredEmmott::GUI::skia_text_fallback_detail {

bool HasColorGlyphTables(SkTypeface* const tf) noexcept {
  if (!tf) {
    return false;
  }
  constexpr auto colr = SkSetFourByteTag('C', 'O', 'L', 'R');
  constexpr auto cbdt = SkSetFourByteTag('C', 'B', 'D', 'T');
  constexpr auto sbix = SkSetFourByteTag('s', 'b', 'i', 'x');
  return tf->getTableSize(colr) > 0 || tf->getTableSize(cbdt) > 0
    || tf->getTableSize(sbix) > 0;
}

void ForEachRun(
  const SkFont& mainFont,
  const std::string_view text,
  SkFontMgr* const fontMgr,
  const std::function<void(const Run&)>& cb) {
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
  std::unordered_map<UChar32, sk_sp<SkTypeface>> fallbacks;

  const auto typefaceFor = [&](const UChar32 cp) -> SkTypeface* {
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
    // codepoint (e.g. Segoe UI Symbol, Noto Emoji) before reaching a color
    // emoji font. If the codepoint defaults to emoji presentation per
    // Unicode (UCHAR_EMOJI_PRESENTATION) and the first match is monochrome,
    // retry through the "emoji" generic family — fontconfig maps that to
    // Noto Color Emoji / Segoe UI Emoji on Linux. Keep the original match
    // if no color font is found.
    if (
      tf && u_hasBinaryProperty(cp, UCHAR_EMOJI_PRESENTATION)
      && !HasColorGlyphTables(tf.get())) {
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

  const auto* const bytes = reinterpret_cast<const uint8_t*>(text.data());
  const auto length = static_cast<int32_t>(text.size());

  int32_t i = 0;
  size_t runStart = 0;
  SkTypeface* runTypeface = nullptr;

  while (i < length) {
    const int32_t cpStart = i;
    UChar32 cp;
    U8_NEXT_OR_FFFD(bytes, i, length, cp);
    SkTypeface* const tf = typefaceFor(cp);

    if (runTypeface == nullptr) {
      runTypeface = tf;
      runStart = static_cast<size_t>(cpStart);
      continue;
    }
    if (runTypeface == tf) {
      continue;
    }
    // Boundary: flush the current run.
    const auto flushEnd = static_cast<size_t>(cpStart);
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
