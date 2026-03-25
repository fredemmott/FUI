// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "NavigationViewBackButton.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr std::string_view BackGlyph = "\ue72b";
}// namespace

NavigationViewBackButton::NavigationViewBackButton(Window* const window)
  : NavigationViewButton(window, BackGlyph) {
  // Disabled until something is pushed on the stack
  this->SetIsDirectlyDisabled(true);
}

NavigationViewBackButton::~NavigationViewBackButton() = default;

void NavigationViewBackButton::Invoke() {
  this->MarkActivated();
}

}// namespace FredEmmott::GUI::Widgets