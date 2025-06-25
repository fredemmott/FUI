// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/config.hpp>

/// X(NAME, WEIGHT, ...typefaces)
#define FUI_ENUM_SYSTEM_FONT_TYPEFACES(X) \
  X(Regular, Normal, "Segoe UI Variable Text", "Segoe UI") \
  X(BodyStrong, SemiBold, "Segoe UI Variable Text", "Segoe UI") \
  X(Caption, Normal, "Segoe UI Variable Small") \
  X(Display, SemiBold, "Segoe UI Variable Display", "Segoe UI") \
  X(Glyph, Normal, "Segoe Fluent Icons", "Segoe MDL2 Assets")

/// X(USAGE, TYPEFACE)
#define FUI_ENUM_SYSTEM_FONT_FONTS(X) \
  X(Caption, Caption) \
  X(Body, Regular) \
  X(BodyStrong, BodyStrong) \
  X(BodyLarge, Regular) \
  X(Subtitle, Display) \
  X(Title, Display) \
  X(TitleLarge, Display) \
  X(Display, Display)

namespace FredEmmott::GUI::SystemFont {

#ifdef FUI_ENABLE_SKIA
Font ResolveSkiaFont(Usage) noexcept;
Font ResolveGlyphSkiaFont(Usage) noexcept;
#endif

#ifdef FUI_ENABLE_DIRECT2D
Font ResolveDirectWriteFont(Usage) noexcept;
Font ResolveGlyphDirectWriteFont(Usage) noexcept;
#endif

}// namespace FredEmmott::GUI::SystemFont