// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

// These are only kept here because the WinUI themes reference them - however,
// they're almost all fixed values on Windows 10 and above.
//
// Use accent colors or the StaticTheme colors instead.
#define FUI_WINAPI_SYS_COLORS(X) \
  X(SystemColorActiveCaptionColor, COLOR_ACTIVECAPTION) \
  X(SystemColorBackgroundColor, COLOR_BACKGROUND) \
  X(SystemColorButtonFaceColor, COLOR_BTNFACE) \
  X(SystemColorButtonTextColor, COLOR_BTNTEXT) \
  X(SystemColorCaptionTextColor, COLOR_CAPTIONTEXT) \
  X(SystemColorGrayTextColor, COLOR_GRAYTEXT) \
  X(SystemColorHighlightColor, COLOR_HIGHLIGHT) \
  X(SystemColorHighlightTextColor, COLOR_HIGHLIGHTTEXT) \
  X(SystemColorHotlightColor, COLOR_HOTLIGHT) \
  X(SystemColorInactiveCaptionColor, COLOR_INACTIVECAPTION) \
  X(SystemColorInactiveCaptionTextColor, COLOR_INACTIVECAPTIONTEXT) \
  X(SystemColorWindowColor, COLOR_WINDOW) \
  X(SystemColorWindowTextColor, COLOR_WINDOWTEXT) \
  X(SystemColorDisabledTextColor, COLOR_GRAYTEXT)

#define FUI_WINRT_UI_ACCENT_COLORS(X) \
  X(SystemAccentColorDark3, UIColorType::AccentDark3) \
  X(SystemAccentColorDark2, UIColorType::AccentDark2) \
  X(SystemAccentColorDark1, UIColorType::AccentDark1) \
  X(SystemAccentColor, UIColorType::Accent) \
  X(SystemAccentColorLight1, UIColorType::AccentLight1) \
  X(SystemAccentColorLight2, UIColorType::AccentLight2) \
  X(SystemAccentColorLight3, UIColorType::AccentLight3)

namespace FredEmmott::GUI {
class Color;
}// namespace FredEmmott::GUI

/** The 'System Theme' is the currently selected Windows theme.
 *
 * This can be customized by the user, and can change at runtime.
 *
 * Changing the System Theme may also cause the current Static Theme to change.
 */
namespace FredEmmott::GUI::SystemTheme {

// This library is a visual clone of WinUI3, so even if we're ported to
// another platform, we're going to base the color usages off of the
// Windows theme colors (Windows::UI::ViewManagement::UIColorType)
enum class ColorType {
#define X(KEY, IMPL) KEY,
  FUI_WINAPI_SYS_COLORS(X) FUI_WINRT_UI_ACCENT_COLORS(X)
#undef X
};
using enum ColorType;

Color Resolve(ColorType usage) noexcept;

/** Purge caches and update to the current Windows theme.
 *
 * You probably want to call `StaticTheme::Refresh()` instead, which will call
 * this for you.
 */
void Refresh();

}// namespace FredEmmott::GUI::SystemTheme