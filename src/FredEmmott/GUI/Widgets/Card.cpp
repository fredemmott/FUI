// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

namespace FredEmmott::GUI::Widgets {

Card::Card(std::size_t id, const Options& options)
  : Widget(id), mOptions(options) {
  YGNodeStyleSetPadding(GetLayoutNode(), YGEdgeAll, Spacing * 4);
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

Widget* Card::GetChild() const noexcept {
  return mChild.get();
}

void Card::PaintOwnContent(SkCanvas* canvas, const Style& style) const {
  const auto x = YGNodeLayoutGetLeft(this->GetLayoutNode());
  const auto y = YGNodeLayoutGetTop(this->GetLayoutNode());
  const auto w = YGNodeLayoutGetWidth(this->GetLayoutNode());
  const auto h = YGNodeLayoutGetHeight(this->GetLayoutNode());

  SkPaint paint;
  paint.setColor(mOptions.mBackgroundColor);
  paint.setAntiAlias(true);
  canvas->drawRoundRect(
    SkRect::MakeXYWH(x, y, w, h), Spacing * 2, Spacing * 2, paint);
}

}// namespace FredEmmott::GUI::Widgets