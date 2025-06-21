// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StaticTheme.hpp"

#include <Windows.h>
#include <winrt/windows.ui.viewmanagement.h>

#include <FredEmmott/GUI/StaticTheme/Common.hpp>

#include "Color.hpp"
#include "SystemSettings.hpp"
#include "SystemTheme.hpp"

using namespace winrt::Windows::UI::ViewManagement;

namespace FredEmmott::GUI::StaticTheme {
namespace {

std::optional<Theme> gThemeKind;

}// namespace

Theme GetCurrent() {
  if (gThemeKind) {
    return gThemeKind.value();
  }

  gThemeKind = []() {
    using enum Theme;

    if (SystemSettings::Get().GetHighContrast()) {
      return HighContrast;
    }

    // Microsoft's recommended approach for detecting dark mode...
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

}// namespace FredEmmott::GUI::StaticTheme