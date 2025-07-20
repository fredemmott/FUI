// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

namespace FredEmmott::GUI::Widgets {

namespace {
const auto CardStyleClass = StyleClass::Make("Card");
}

Card::Card(std::size_t id) : Widget(id, {CardStyleClass}) {
  using namespace StaticTheme::Common;
  static const auto CardStyle
    = Style()
        .BackgroundColor(CardBackgroundFillColorDefaultBrush)
        .BorderRadius(OverlayCornerRadius)
        .Padding(Spacing * 4);
  BuiltInStyles() = CardStyle;
}

}// namespace FredEmmott::GUI::Widgets