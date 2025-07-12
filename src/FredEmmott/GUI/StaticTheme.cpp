// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StaticTheme.hpp"

#include <Windows.h>
#include <wil/winrt.h>
#include <windows.ui.viewmanagement.h>

#include <FredEmmott/GUI/StaticTheme/Common.hpp>

#include "Color.hpp"
#include "SystemSettings.hpp"
#include "SystemTheme.hpp"
#include "detail/win32_detail.hpp"

using namespace FredEmmott::GUI::win32_detail;

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
    using namespace ABI::Windows::UI::ViewManagement;
    const auto settings = wil::ActivateInstance<IUISettings3>(
      RuntimeClass_Windows_UI_ViewManagement_UISettings);
    ABI::Windows::UI::Color color {};
    CheckHResult(settings->GetColorValue(UIColorType_Foreground, &color));
    const auto [a, r, g, b] = color;
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