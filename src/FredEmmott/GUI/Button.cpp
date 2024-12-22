// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <skia/core/SkRRect.h>

namespace FredEmmott::GUI {

Button::Button(std::size_t id, const Options& options, Widget* label)
  : Widget(id), mOptions(options), mLabel(label) {
  YGNodeInsertChild(this->GetLayoutNode(), label->GetLayoutNode(), 0);
  this->SetLayoutConstraints();
}

void Button::SetLayoutConstraints() {
  const auto self = this->GetLayoutNode();
  const auto label = mLabel->GetLayoutNode();
  YGNodeStyleSetPadding(self, YGEdgeHorizontal, Spacing * 3);
  YGNodeStyleSetPadding(self, YGEdgeVertical, Spacing);

  // TODO (Yoga 3.3+ ?): use YGNodeStyle*FitContent()
  YGNodeStyleSetBoxSizing(self, YGBoxSizingContentBox);
  auto childWidth = YGNodeStyleGetMaxWidth(label);

  if (childWidth.unit == YGUnitUndefined) {
    childWidth = YGNodeStyleGetWidth(label);
  }
  switch (childWidth.unit) {
    case YGUnitPercent:
      YGNodeStyleSetMaxWidthPercent(self, childWidth.value);
      break;
    case YGUnitPoint:
      YGNodeStyleSetMaxWidth(self, childWidth.value);
      break;
    default:
      break;
  }
}

void Button::Paint(SkCanvas* canvas) const {
  const auto l = this->GetLayoutNode();
  const auto x = YGNodeLayoutGetLeft(l);
  const auto y = YGNodeLayoutGetTop(l);
  const auto w = YGNodeLayoutGetWidth(l);
  const auto h = YGNodeLayoutGetHeight(l);

  const auto button = SkRRect::MakeRectXY(SkRect::MakeXYWH(x, y, w, h), Spacing, Spacing);

  SkPaint paint;
  paint.setColor(mOptions.mFillColor);
  paint.setAntiAlias(true);
  canvas->drawRRect(button, paint);

  paint.setColor(mOptions.mBorderColor);
  paint.setStyle(SkPaint::kStroke_Style);
  const auto borderWidth = Spacing/4;
  SkRRect border {};
  button.inset(borderWidth / 2.0, borderWidth / 2.0, &border);
  paint.setStrokeWidth(borderWidth);
  canvas->drawRRect(border , paint);

  canvas->save();
  canvas->translate(x, y);
  mLabel->Paint(canvas);
  canvas->restore();
}

}// namespace FredEmmott::GUI
