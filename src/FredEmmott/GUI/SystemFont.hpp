// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkFontMgr.h>

namespace FredEmmott::GUI {
class Font;
}

namespace FredEmmott::GUI::SystemFont {

// https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography
enum class Usage {
  Caption,
  Body,
  BodyStrong,
  BodyLarge,
  Subtitle,
  Title,
  TitleLarge,
  Display,
};
using enum Usage;

Font Resolve(Usage) noexcept;

Font ResolveGlyphFont(Usage) noexcept;

sk_sp<SkFontMgr> GetFontManager() noexcept;

}// namespace FredEmmott::GUI::SystemFont