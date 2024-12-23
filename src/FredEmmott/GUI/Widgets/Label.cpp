// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/utility/lazy_init.hpp>

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

Label::Label(std::size_t id) : Widget(id) {
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &Label::Measure);
}

void Label::SetText(std::string_view text) {
  if (text == mText) {
    return;
  }
  mText = std::string {text};
  YGNodeMarkDirty(this->GetLayoutNode());
}

void Label::PaintOwnContent(SkCanvas* canvas, const Style& style) const {
  const auto l = this->GetLayoutNode();
  const auto& font = *style.mFont;

  SkPaint paint;
  paint.setStyle(SkPaint::kFill_Style);
  paint.setColor(*style.mColor);

  SkFontMetrics metrics {};
  font.GetMetricsInPixels(&metrics);

  const auto x = YGNodeLayoutGetLeft(l);
  const auto top = YGNodeLayoutGetTop(l);
  const auto height = YGNodeLayoutGetHeight(l) - metrics.fDescent;
  const auto y = top + height;

  canvas->drawString(mText.c_str(), x, y, font, paint);
}

WidgetStyles Label::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mDefault = {
      .mColor = SystemColor::Foreground,
      .mFont = SystemFont::Body,
    },
  };
  return ret;
}

void Label::OnComputedStyleChange(const Style& style) {
  if (mFont != style.mFont) {
    mFont = style.mFont.value();
    YGNodeMarkDirty(this->GetLayoutNode());
  }
}

YGSize Label::Measure(
  YGNodeConstRef node,
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto self = static_cast<Label*>(YGNodeGetContext(node));

  const auto& font = self->mFont;
  const auto& text = self->mText;

  return {
    font->measureText(text.data(), text.size(), SkTextEncoding::kUTF8),
    font.GetHeightInPixels(),
  };
}

}// namespace FredEmmott::GUI::Widgets