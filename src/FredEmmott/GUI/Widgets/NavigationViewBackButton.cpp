// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "NavigationViewBackButton.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr std::string_view BackGlyph = "\ue72b";
}// namespace

NavigationViewBackButton::NavigationViewBackButton()
  : NavigationViewButton(BackGlyph) {
  // Disabled until something is pushed on the stack
  this->SetIsDirectlyDisabled(true);
}

NavigationViewBackButton::~NavigationViewBackButton() = default;

void NavigationViewBackButton::Invoke() {
  mWasActivated = true;
}

}// namespace FredEmmott::GUI::Widgets