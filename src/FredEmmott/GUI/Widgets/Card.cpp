// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

namespace FredEmmott::GUI::Widgets {

Card::Card(std::size_t id, const Options& options, Widget* child)
  : Widget(id), mOptions(options), mChild(child) {
  YGNodeInsertChild(this->GetLayoutNode(), child->GetLayoutNode(), 0);
}

void Card::Paint(SkCanvas* canvas) const {
  const auto x = YGNodeLayoutGetLeft(this->GetLayoutNode());
  const auto y = YGNodeLayoutGetTop(this->GetLayoutNode());
  const auto w = YGNodeLayoutGetWidth(this->GetLayoutNode());
  const auto h = YGNodeLayoutGetHeight(this->GetLayoutNode());

  SkPaint paint;
  paint.setColor(mOptions.mBackgroundColor);
  paint.setAntiAlias(true);
  canvas->drawRoundRect(
    SkRect::MakeXYWH(x, y, w, h), Spacing * 2, Spacing * 2, paint);

  canvas->save();
  canvas->translate(x, y);
  mChild->Paint(canvas);
  canvas->restore();
}

}// namespace FredEmmott::GUI::Widgets