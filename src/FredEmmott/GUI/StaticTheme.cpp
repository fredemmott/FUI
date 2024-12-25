// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StaticTheme.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/detail/WinUI3Themes/Macros.hpp>
#include <FredEmmott/GUI/detail/WinUI3Themes/Themes.hpp>

#include "Brush.hpp"
#include "Color.hpp"
#include "SystemTheme.hpp"

namespace FredEmmott::GUI::StaticTheme {
using namespace gui_detail::WinUI3Themes;

namespace {

std::optional<ThemeKind> gThemeKind;

const Theme& GetCurrentTheme() {
  using enum ThemeKind;
  switch (GetCurrentThemeKind()) {
    case Dark:
      return DefaultTheme;
    case Light:
      return LightTheme;
    case HighContrast:
      return HighContrastTheme;
  }
  std::unreachable();
}

}// namespace

Color Resolve(const ColorType color) noexcept {
  using namespace gui_detail::WinUI3Themes;
  const auto& theme = GetCurrentTheme();
  switch (color) {
#define DEFINE_CASE(X) \
  case ColorType::X: \
    return theme.m##X;
    FUI_WINUI_THEME_COLORS(DEFINE_CASE)
#undef DEFINE_CASE
  }
  std::unreachable();
}

Brush Resolve(const BrushType brush) noexcept {
  using namespace gui_detail::WinUI3Themes;
  const auto& theme = GetCurrentTheme();
  switch (brush) {
#define DEFINE_CASE(X) \
  case X: \
    return theme.m##X;
    FUI_WINUI_THEME_BRUSHES(DEFINE_CASE)
#undef DEFINE_CASE
  }
  std::unreachable();
}

ThemeKind GetCurrentThemeKind() {
  if (gThemeKind) {
    return gThemeKind.value();
  }

  gThemeKind = []() {
    using enum ThemeKind;

    HIGHCONTRAST contrast {sizeof(HIGHCONTRAST)};
    if (SystemParametersInfo(
          SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0)) {
      if ((contrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON) {
        return HighContrast;
      }
    }

    // Microsoft's recommend approach for detecting dark mode...
    const auto foreground
      = SystemTheme::Resolve(SystemTheme::SystemColorWindowTextColor);
    const bool isForegroundLight
      = (((5 * SkColorGetG(foreground)) + (2 * SkColorGetR(foreground)) + SkColorGetB(foreground)) > (8 * 128));

    return isForegroundLight ? Dark : Light;
  }();

  return gThemeKind.value();
}

void Refresh() {
  gThemeKind = std::nullopt;
  SystemTheme::Refresh();

  // Re-populate gThemeKind
  (void)GetCurrentThemeKind();
}

}// namespace FredEmmott::GUI::StaticTheme