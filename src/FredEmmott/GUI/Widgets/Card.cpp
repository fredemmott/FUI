// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

namespace FredEmmott::GUI::Widgets {
Card::Card(std::size_t id) : Widget(id) {
}

WidgetStyles Card::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mBase = {
      .mBackgroundColor = WidgetColor::CardBackgroundFillDefault,
      .mBorderRadius = Spacing * 2,
      .mMargin = Spacing * 9,
      .mPadding = Spacing * 4,
    },
  };
  return ret;
}

}// namespace FredEmmott::GUI::Widgets