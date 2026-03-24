// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

#include "FredEmmott/GUI/Window.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass CardStyleClass {"Card"};
auto& CardStyle() {
  using namespace StaticTheme::Common;
  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(CardBackgroundFillColorDefaultBrush)
      .BorderRadius(OverlayCornerRadius)
      .Padding(16),
  };
  return ret;
}
}// namespace

Card::Card(Window* const window)
  : Widget(window, CardStyleClass, CardStyle()) {}

Card::~Card() = default;

}// namespace FredEmmott::GUI::Widgets