// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

namespace FredEmmott::GUI::Widgets {
Card::Card(std::size_t id) : Widget(id) {
  YGNodeStyleSetMargin(GetLayoutNode(), YGEdgeAll, Spacing * 9);
}

void Card::SetChild(Widget* child) {
  if (child == mChild.get()) {
    return;
  }

  if (mChild) {
    YGNodeRemoveChild(this->GetLayoutNode(), mChild->GetLayoutNode());
  }
  mChild.reset(child);
  YGNodeInsertChild(this->GetLayoutNode(), child->GetLayoutNode(), 0);
}

std::span<Widget* const> Card::GetChildren() const noexcept {
  if (!mChild) {
    return {};
  }
  mChildRawPointer = mChild.get();
  return {&mChildRawPointer, 1};
}
WidgetStyles Card::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mDefault = {
      .mBackgroundColor = WidgetColor::CardBackgroundFillDefault,
      .mBorderRadius = Spacing * 2,
      .mPadding = Spacing * 4,
    },
  };
  return ret;
}

Widget* Card::GetChild() const noexcept {
  return mChild.get();
}

}// namespace FredEmmott::GUI::Widgets