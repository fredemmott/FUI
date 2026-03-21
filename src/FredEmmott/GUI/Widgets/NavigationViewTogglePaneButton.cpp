// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationViewTogglePaneButton.hpp"

#include "NavigationView.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
// a.k.a. Hamburger Menu
constexpr auto GlobalNavButtonGlyph = "\ue700";
}// namespace

NavigationViewTogglePaneButton::NavigationViewTogglePaneButton(
  const id_type id,
  NavigationView* nav)
  : NavigationViewButton(id, GlobalNavButtonGlyph),
    mNavigationView(nav) {}

NavigationViewTogglePaneButton::~NavigationViewTogglePaneButton() = default;

void NavigationViewTogglePaneButton::Invoke() {
  mNavigationView->TogglePaneIsExpanded();
}

}// namespace FredEmmott::GUI::Widgets
