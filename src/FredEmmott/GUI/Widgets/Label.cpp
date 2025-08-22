// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme::Generic;

namespace {
constexpr LiteralStyleClass LabelStyleClass("Label");
}

Label::Label(
  std::size_t id,
  const std::optional<ImmutableStyle>& style,
  const StyleClasses& classes)
  : Widget(
      id,
      style.value_or(TextBlockClassStyles()),
      classes + LabelStyleClass) {
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &Label::Measure);
  YGNodeSetNodeType(this->GetLayoutNode(), YGNodeTypeText);
}

Widget* Label::SetText(std::string_view text) {
  if (text == mText) {
    return this;
  }
  mText = std::string {text};

  if (!mFont) {
    return this;
  }

  // Check before calling `YGNodeMarkDirty()` as this will mark
  // all ancestor nodes as dirty, even if this node layout/size doesn't change
  const auto yoga = this->GetLayoutNode();
  const auto availableWidth = YGNodeLayoutGetWidth(yoga)
    - (YGNodeLayoutGetBorder(yoga, YGEdgeLeft)
       + YGNodeLayoutGetPadding(yoga, YGEdgeLeft)
       + YGNodeLayoutGetPadding(yoga, YGEdgeRight)
       + YGNodeLayoutGetBorder(yoga, YGEdgeRight));
  if (std::abs(mFont.MeasureTextWidth(mText) - availableWidth) > 1.0) {
    YGNodeMarkDirty(this->GetLayoutNode());
  }

  return this;
}

void Label::PaintOwnContent(
  Renderer* renderer,
  const Rect& outerRect,
  const Style& style) const {
#ifndef NDEBUG
  if (style.Font() != mFont) [[unlikely]] {
    throw std::logic_error(
      "Stylesheet font does not match mFont; computed style not updated");
  }
#endif
  const auto yoga = this->GetLayoutNode();
  const Rect rect = outerRect.WithInset(
    YGNodeLayoutGetPadding(yoga, YGEdgeLeft)
      + YGNodeLayoutGetBorder(yoga, YGEdgeLeft),
    YGNodeLayoutGetPadding(yoga, YGEdgeTop)
      + YGNodeLayoutGetBorder(yoga, YGEdgeTop),
    YGNodeLayoutGetPadding(yoga, YGEdgeRight)
      + YGNodeLayoutGetBorder(yoga, YGEdgeRight),
    YGNodeLayoutGetPadding(yoga, YGEdgeBottom)
      + YGNodeLayoutGetBorder(yoga, YGEdgeBottom));

  const auto metrics = mFont.GetMetrics();
  Point baseline {
    rect.GetLeft(),
    rect.GetBottom() - metrics.mDescent,
  };

  switch (style.TextAlign().value_or(TextAlign::Left)) {
    case TextAlign::Left:
      break;
    case TextAlign::Center: {
      const auto textWidth = mFont.MeasureTextWidth(mText);
      baseline.mX += (rect.GetWidth() - textWidth) / 2;
      break;
    }
    case TextAlign::Right: {
      const auto textWidth = mFont.MeasureTextWidth(mText);
      baseline.mX = (rect.GetRight() - textWidth);
      break;
    }
  }

  renderer->DrawText(style.Color().value(), rect, mFont, mText, baseline);
}

Widget::ComputedStyleFlags Label::OnComputedStyleChange(
  const Style& style,
  StateFlags) {
  if (mFont != style.Font()) {
    mFont = style.Font().value();
    YGNodeMarkDirty(this->GetLayoutNode());
  }

  return ComputedStyleFlags::Empty;
}

YGSize Label::Measure(
  YGNodeConstRef node,
  [[maybe_unused]] float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  [[maybe_unused]] float height,
  [[maybe_unused]] YGMeasureMode heightMode) {
  const auto self = static_cast<Label*>(FromYogaNode(node));

  const auto& font = self->mFont;
  const auto& text = self->mText;

  const auto tw = font.MeasureTextWidth(text);

  return {
    tw,
    -font.GetMetrics().mAscent,
  };
}

}// namespace FredEmmott::GUI::Widgets