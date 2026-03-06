// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationViewTogglePaneButton.hpp"

#include "NavigationView.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
// "GlobalNavButton", a.k.a 'Hamburger'
constexpr auto Glyph = "\ue700";
}// namespace

NavigationViewTogglePaneButton::NavigationViewTogglePaneButton(
  const id_type id,
  NavigationView* nav)
  : NavigationViewButton(id, Glyph),
    mNavigationView(nav) {}

NavigationViewTogglePaneButton::~NavigationViewTogglePaneButton() = default;

void NavigationViewTogglePaneButton::Invoke() {
  mNavigationView->TogglePaneIsExpanded();
}

void NavigationViewTogglePaneButton::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  this->PaintCenteredGlyph(renderer, rect, style.Color().value());
}

}// namespace FredEmmott::GUI::Widgets
