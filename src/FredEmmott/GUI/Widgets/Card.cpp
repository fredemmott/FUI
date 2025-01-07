// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

namespace FredEmmott::GUI::Widgets {
Card::Card(std::size_t id) : Widget(id) {
}

WidgetStyles Card::GetBuiltInStyles() const {
  using namespace StaticTheme::Common;
  static const WidgetStyles ret {
    .mBase = {
      .mBackgroundColor = CardBackgroundFillColorDefaultBrush,
      .mBorderRadius = OverlayCornerRadius,
      .mMargin = Spacing * 9,
      .mPadding = Spacing * 4,
    },
  };
  return ret;
}

}// namespace FredEmmott::GUI::Widgets