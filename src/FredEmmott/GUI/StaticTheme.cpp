// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "StaticTheme.hpp"

#include <Windows.h>
#include <wil/winrt.h>
#include <windows.ui.viewmanagement.h>

#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <stack>

#include "Color.hpp"
#include "SystemSettings.hpp"
#include "SystemTheme.hpp"
#include "detail/win32_detail.hpp"

using namespace FredEmmott::GUI::win32_detail;

namespace FredEmmott::GUI::StaticTheme {
namespace {

thread_local std::optional<Theme> gSystemTheme;
thread_local std::stack<std::optional<Theme>> gOverrideStack;

[[nodiscard]]
Theme GetSystemTheme() {
  if (gSystemTheme) {
    return *gSystemTheme;
  }

  gSystemTheme = []() {
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

  return gSystemTheme.value();
}

}// namespace

namespace static_theme_detail {
void PushOverride(const std::optional<Theme> theme) {
  gOverrideStack.push(theme);
}

void PopOverride() {
  gOverrideStack.pop();
}

}// namespace static_theme_detail

Theme GetCurrent() {
  const auto configuredTheme
    = gOverrideStack.empty() ? gSystemTheme : gOverrideStack.top();
  if (configuredTheme) {
    return *configuredTheme;
  }
  return GetSystemTheme();
}

void Refresh() {
  gSystemTheme = std::nullopt;
  // Re-populate gThemeKind
  std::ignore = GetSystemTheme();

  // Load specific colors
  SystemTheme::Refresh();
}

}// namespace FredEmmott::GUI::StaticTheme