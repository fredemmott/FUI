// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

namespace FredEmmott::GUI {

Button::Button(std::size_t id, const Options&, Widget* label)
  : Widget(id), mLabel(label) {
  YGNodeInsertChild(this->GetLayoutNode(), label->GetLayoutNode(), 0);
  this->SetLayoutConstraints();
}

void Button::SetLayoutConstraints() {
  const auto l = this->GetLayoutNode();
  YGNodeStyleSetPadding(l, YGEdgeAll, Spacing * 2);
}

void Button::Paint(SkCanvas* canvas) const {
  const auto l = this->GetLayoutNode();
  const auto x = YGNodeLayoutGetLeft(l);
  const auto y = YGNodeLayoutGetTop(l);

  canvas->save();
  canvas->translate(x, y);
  mLabel->Paint(canvas);
  canvas->restore();
}

}// namespace FredEmmott::GUI
