// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#define FUI_WINAPI_SYS_COLORS(X) \
  X(ActiveCaptionColor, COLOR_ACTIVECAPTION) \
  X(BackgroundColor, COLOR_BACKGROUND) \
  X(ButtonFaceColor, COLOR_BTNFACE) \
  X(ButtonTextColor, COLOR_BTNTEXT) \
  X(CaptionTextColor, COLOR_CAPTIONTEXT) \
  X(GrayTextColor, COLOR_GRAYTEXT) \
  X(HighlightColor, COLOR_HIGHLIGHT) \
  X(HighlightTextColor, COLOR_HIGHLIGHTTEXT) \
  X(HotlightColor, COLOR_HOTLIGHT) \
  X(InactiveCaptionColor, COLOR_INACTIVECAPTION) \
  X(InactiveCaptionTextColor, COLOR_INACTIVECAPTIONTEXT) \
  X(WindowColor, COLOR_WINDOW) \
  X(WindowTextColor, COLOR_WINDOWTEXT) \
  X(DisabledTextColor, COLOR_GRAYTEXT)

#define FUI_WINRT_UI_ACCENT_COLORS(X) \
  X(AccentColorDark3, UIColorType::AccentDark3) \
  X(AccentColorDark2, UIColorType::AccentDark2) \
  X(AccentColorDark1, UIColorType::AccentDark1) \
  X(AccentColor, UIColorType::Accent) \
  X(AccentColorLight1, UIColorType::AccentLight1) \
  X(AccentColorLight2, UIColorType::AccentLight2) \
  X(AccentColorLight3, UIColorType::AccentLight3) \
  X(ForegroundColor, \
    UIColorType::Foreground)// FIXME: not used by WinUI3 - captionText?

namespace FredEmmott::GUI {
class Color;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::SystemColor {

// This library is a visual clone of WinUI3, so even if we're ported to
// another platform, we're going to base the color usages off of the
// Windows theme colors (Windows::UI::ViewManagement::UIColorType)
enum class Usage {
#define X(KEY, IMPL) System##KEY,
  FUI_WINAPI_SYS_COLORS(X) FUI_WINRT_UI_ACCENT_COLORS(X)
#undef X
};
using enum Usage;

Color Resolve(Usage usage) noexcept;

}// namespace FredEmmott::GUI::SystemColor