// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

namespace {
const auto LabelStyleClass = StyleClass::Make("Label");
}

Label::Label(std::size_t id, const StyleClasses& classes)
  : Widget(id, classes + LabelStyleClass) {
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &Label::Measure);
  YGNodeSetNodeType(this->GetLayoutNode(), YGNodeTypeText);
}

void Label::SetText(std::string_view text) {
  if (text == mText) {
    return;
  }
  mText = std::string {text};
  YGNodeMarkDirty(this->GetLayoutNode());
}

void Label::PaintOwnContent(
  SkCanvas* canvas,
  const Rect& rect,
  const Style& style) const {
#ifndef NDEBUG
  if (style.mFont != mFont) [[unlikely]] {
    throw std::logic_error(
      "Stylesheet font does not match mFont; computed style not updated");
  }
#endif

  auto paint = style.mColor->GetSkiaPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);

  const auto metrics = mFont.GetMetrics();

  canvas->drawString(
    mText.c_str(),
    rect.GetLeft(),
    rect.GetBottom() - metrics.mDescent,
    mFont,
    paint);
}

Widget::ComputedStyleFlags Label::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  if (mFont != style.mFont) {
    mFont = style.mFont.value();
    YGNodeMarkDirty(this->GetLayoutNode());
  }

  return ComputedStyleFlags::Empty;
}

YGSize Label::Measure(
  YGNodeConstRef node,
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto self = static_cast<Label*>(FromYogaNode(node));

  const auto& font = self->mFont;
  const auto& text = self->mText;

  return {
    font.MeasureTextWidth(text),
    font.GetMetrics().mSize,
  };
}

}// namespace FredEmmott::GUI::Widgets