// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "AppBarButton.hpp"

#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass AppBarButtonStyleClass {"AppBarButton"};
constexpr LiteralStyleClass AppBarButtonBorderContainerStyleClass {
  "AppBarButton/Border"};
constexpr LiteralStyleClass AppBarButtonContentContainerStyleClass {
  "AppBarButton/Content"};
const auto& AppBarButtonStyle() {
  static const ImmutableStyle ret {
    Style().Height(48).MinHeight(48).MinWidth(48)};
  return ret;
}

}// namespace

AppBarButton::AppBarButton(Window* const window)
  : Button(window, AppBarButtonStyleClass, AppBarButtonStyle()) {
  this->SetStructuralChildren({
    mBorderContainer
    = new Widget(window, AppBarButtonBorderContainerStyleClass, {}),
  });
  mBorderContainer->SetStructuralChildren({
    mContentContainer
    = new Widget(window, AppBarButtonContentContainerStyleClass, {}),
  });
  mContentContainer->SetStructuralChildren({
    mGlyph = new Label(window),
    mLabel = new Label(window),
    mChevron = new Label(window),
  });
}

AppBarButton::~AppBarButton() = default;

}// namespace FredEmmott::GUI::Widgets