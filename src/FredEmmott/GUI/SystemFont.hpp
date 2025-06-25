// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFontMgr.h>
#endif

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

Font Resolve(Usage);
Font ResolveGlyphFont(Usage);

#ifdef FUI_ENABLE_SKIA
sk_sp<SkFontMgr> GetFontManager() noexcept;
#endif

}// namespace FredEmmott::GUI::SystemFont