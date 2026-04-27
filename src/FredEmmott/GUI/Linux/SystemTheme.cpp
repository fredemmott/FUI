// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux stub. Returns a fixed Light-theme palette so widgets can be styled
// at compile time. A real impl will read xdg-desktop-portal's color-scheme
// and follow the system accent.

#include <FredEmmott/GUI/SystemTheme.hpp>

#include <utility>

#include <FredEmmott/GUI/Color.hpp>

namespace FredEmmott::GUI::SystemTheme {

namespace {

// Windows 11 Light theme accent palette.
constexpr auto Accent = ColorConstant::FromARGB32(0xff, 0x00, 0x78, 0xd4);
constexpr auto AccentLight1 = ColorConstant::FromARGB32(0xff, 0x33, 0x92, 0xda);
constexpr auto AccentLight2 = ColorConstant::FromARGB32(0xff, 0x66, 0xad, 0xe0);
constexpr auto AccentLight3 = ColorConstant::FromARGB32(0xff, 0x99, 0xc8, 0xe7);
constexpr auto AccentDark1 = ColorConstant::FromARGB32(0xff, 0x00, 0x6c, 0xbe);
constexpr auto AccentDark2 = ColorConstant::FromARGB32(0xff, 0x00, 0x5a, 0x9e);
constexpr auto AccentDark3 = ColorConstant::FromARGB32(0xff, 0x00, 0x42, 0x75);

// Classic Win32 sys-color values (GetSysColor) — used by a handful of
// themes; full opacity, light-theme variants.
constexpr auto Black = ColorConstant::FromARGB32(0xff, 0x00, 0x00, 0x00);
constexpr auto White = ColorConstant::FromARGB32(0xff, 0xff, 0xff, 0xff);
constexpr auto Gray = ColorConstant::FromARGB32(0xff, 0x80, 0x80, 0x80);
constexpr auto LightGray = ColorConstant::FromARGB32(0xff, 0xf0, 0xf0, 0xf0);
constexpr auto Highlight = Accent;
constexpr auto Hotlight = ColorConstant::FromARGB32(0xff, 0x00, 0x66, 0xcc);

}// namespace

ColorConstant Resolve(const ColorType usage) noexcept {
  switch (usage) {
    case ColorType::SystemColorActiveCaptionColor:
      return White;
    case ColorType::SystemColorBackgroundColor:
      return LightGray;
    case ColorType::SystemColorButtonFaceColor:
      return LightGray;
    case ColorType::SystemColorButtonTextColor:
      return Black;
    case ColorType::SystemColorCaptionTextColor:
      return Black;
    case ColorType::SystemColorGrayTextColor:
      return Gray;
    case ColorType::SystemColorHighlightColor:
      return Highlight;
    case ColorType::SystemColorHighlightTextColor:
      return White;
    case ColorType::SystemColorHotlightColor:
      return Hotlight;
    case ColorType::SystemColorInactiveCaptionColor:
      return LightGray;
    case ColorType::SystemColorInactiveCaptionTextColor:
      return Gray;
    case ColorType::SystemColorWindowColor:
      return White;
    case ColorType::SystemColorWindowTextColor:
      return Black;
    case ColorType::SystemColorDisabledTextColor:
      return Gray;
    case ColorType::SystemAccentColorDark3:
      return AccentDark3;
    case ColorType::SystemAccentColorDark2:
      return AccentDark2;
    case ColorType::SystemAccentColorDark1:
      return AccentDark1;
    case ColorType::SystemAccentColor:
      return Accent;
    case ColorType::SystemAccentColorLight1:
      return AccentLight1;
    case ColorType::SystemAccentColorLight2:
      return AccentLight2;
    case ColorType::SystemAccentColorLight3:
      return AccentLight3;
  }
  std::unreachable();
}

void Refresh() {
}

}// namespace FredEmmott::GUI::SystemTheme
