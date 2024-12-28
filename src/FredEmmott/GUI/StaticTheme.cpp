// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StaticTheme.hpp"

#include <Windows.h>
#include <winrt/windows.ui.viewmanagement.h>

#include <FredEmmott/GUI/detail/WinUI3Themes/Macros.hpp>
#include <FredEmmott/GUI/detail/WinUI3Themes/Themes.hpp>

#include "Color.hpp"
#include "SystemTheme.hpp"

using namespace winrt::Windows::UI::ViewManagement;

namespace FredEmmott::GUI::StaticTheme {
using namespace gui_detail::WinUI3Themes;

namespace {

std::optional<Theme> gThemeKind;

const auto& GetCurrentThemeData() {
  using enum Theme;
  switch (GetCurrent()) {
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
  const auto& theme = GetCurrentThemeData();
  switch (color) {
#define DEFINE_CASE(X) \
  case ColorType::X: \
    return theme.m##X;
    FUI_WINUI_THEME_COLORS(DEFINE_CASE)
#undef DEFINE_CASE
  }
  std::unreachable();
}

Theme GetCurrent() {
  if (gThemeKind) {
    return gThemeKind.value();
  }

  gThemeKind = []() {
    using enum Theme;

    HIGHCONTRAST contrast {sizeof(HIGHCONTRAST)};
    if (SystemParametersInfo(
          SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0)) {
      if ((contrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON) {
        return HighContrast;
      }
    }

    // Microsoft's recommend approach for detecting dark mode...
    UISettings settings;
    auto [a, r, g, b] = settings.GetColorValue(UIColorType::Foreground);
    const bool isForegroundLight = ((5 * r) + (2 * g) + b) > (8 * 128);

    return isForegroundLight ? Dark : Light;
  }();

  return gThemeKind.value();
}

void Refresh() {
  gThemeKind = std::nullopt;
  SystemTheme::Refresh();

  // Re-populate gThemeKind
  (void)GetCurrent();
}

#define DEFINE_FUI_STATIC_THEME_BRUSH(X) \
  const Resource<Brush> g##X { \
    .mDefault = gui_detail::WinUI3Themes::DefaultTheme.m##X, \
    .mLight = gui_detail::WinUI3Themes::LightTheme.m##X, \
    .mHighContrast = gui_detail::WinUI3Themes::HighContrastTheme.m##X, \
  }; \
  const Resource<Brush>* X = &g##X;
FUI_WINUI_THEME_BRUSHES(DEFINE_FUI_STATIC_THEME_BRUSH)
#undef DEFINE_FUI_STATIC_THEME_BRUSH

}// namespace FredEmmott::GUI::StaticTheme