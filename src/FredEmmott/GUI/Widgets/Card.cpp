// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

namespace FredEmmott::GUI::Widgets {

namespace {
const auto CardStyleClass = StyleClass::Make("Card");
}

Card::Card(std::size_t id) : Widget(id, {CardStyleClass}) {}

Style Card::GetBuiltInStyles() const {
  using namespace StaticTheme::Common;
  static const Style ret {
    .mBackgroundColor = CardBackgroundFillColorDefaultBrush,
    .mBorderRadius = OverlayCornerRadius,
    .mPadding = Spacing * 4,
  };
  return ret;
}

}// namespace FredEmmott::GUI::Widgets