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
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
#ifndef NDEBUG
  if (style.mFont != mFont) [[unlikely]] {
    throw std::logic_error(
      "Stylesheet font does not match mFont; computed style not updated");
  }
#endif

  const auto metrics = mFont.GetMetrics();
  const Point baseline {
    rect.GetLeft(),
    rect.GetBottom() - metrics.mDescent,
  };

  renderer->DrawText(style.mColor.value(), rect, mFont, mText, baseline);
}

Widget::ComputedStyleFlags Label::OnComputedStyleChange(
  const Style& style,
  StateFlags) {
  if (mFont != style.mFont) {
    mFont = style.mFont.value();
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

  return {
    font.MeasureTextWidth(text),
    -font.GetMetrics().mAscent,
  };
}

}// namespace FredEmmott::GUI::Widgets