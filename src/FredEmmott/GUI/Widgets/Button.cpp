// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <skia/core/SkRRect.h>

#include <ranges>

#include "FredEmmott/utility/lazy_init.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

Button::Button(std::size_t id, const Options&) : Widget(id) {
}

Widget* Button::GetChild() const noexcept {
  return mLabel.get();
}

void Button::SetChild(Widget* child) {
  if (child == mLabel.get()) {
    return;
  }

  const auto layout = this->GetLayoutNode();
  if (mLabel) {
    YGNodeRemoveChild(layout, mLabel->GetLayoutNode());
  }
  mLabel.reset(child);
  YGNodeInsertChild(this->GetLayoutNode(), child->GetLayoutNode(), 0);
  this->SetLayoutConstraints();
}

std::span<Widget* const> Button::GetChildren() const noexcept {
  if (!mLabel) {
    return {};
  }

  mLabelRawPointer = mLabel.get();
  return {&mLabelRawPointer, 1};
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

void Button::PaintOwnContent(SkCanvas* canvas, const Style& style) const {
  const auto l = this->GetLayoutNode();
  const auto x = YGNodeLayoutGetLeft(l);
  const auto y = YGNodeLayoutGetTop(l);
  const auto w = YGNodeLayoutGetWidth(l);
  const auto h = YGNodeLayoutGetHeight(l);

  const auto button
    = SkRRect::MakeRectXY(SkRect::MakeXYWH(x, y, w, h), Spacing, Spacing);

  const auto fillColor = *style.mBackgroundColor;
  const auto borderColor = *style.mBorderColor;

  SkPaint paint;
  paint.setColor(fillColor);
  paint.setAntiAlias(true);
  canvas->drawRRect(button, paint);

  paint.setColor(borderColor);
  paint.setStyle(SkPaint::kStroke_Style);
  const auto borderWidth = Spacing / 4;
  SkRRect border {};
  button.inset(borderWidth / 2.0, borderWidth / 2.0, &border);
  paint.setStrokeWidth(borderWidth);
  canvas->drawRRect(border, paint);
}

WidgetStyles Button::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mDefault = {
      .mBackgroundColor = WidgetColor::ControlFillDefault,
      .mBorderColor = WidgetColor::ControlElevationBorder,
      .mBorderRadius = Spacing,
      .mBorderWidth = Spacing / 4,
    },
    .mHover = {
      .mBackgroundColor = SK_ColorRED,
    },
  };
  return ret;
}

}// namespace FredEmmott::GUI::Widgets
