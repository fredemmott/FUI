// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Linux stub. Always reports Light theme; no live switching. A real impl
// will subscribe to xdg-desktop-portal's Settings signal for color-scheme.

#include <FredEmmott/GUI/StaticTheme.hpp>

#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <optional>
#include <stack>

#include <FredEmmott/GUI/SystemTheme.hpp>

namespace FredEmmott::GUI::StaticTheme {
namespace {

thread_local std::stack<std::optional<Theme>> gOverrideStack;

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
  if (!gOverrideStack.empty()) {
    if (const auto& top = gOverrideStack.top()) {
      return *top;
    }
  }
  return Theme::Light;
}

void Refresh() {
  SystemTheme::Refresh();
}

}// namespace FredEmmott::GUI::StaticTheme
