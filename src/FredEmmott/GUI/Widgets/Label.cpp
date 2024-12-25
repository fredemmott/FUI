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
#ifndef NDEBUG
  if (style.mFont != mFont) [[unlikely]] {
    throw std::logic_error(
      "Stylesheet font does not match mFont; computed style not updated");
  }
#endif
  SkFontMetrics metrics {};
  mFont.GetMetricsInPixels(&metrics);

  const auto l = this->GetLayoutNode();
  // TODO: pass rect into PaintOwnContent as we're calculating it anyway
  const auto x = YGNodeLayoutGetLeft(l);
  const auto y = YGNodeLayoutGetTop(l);
  const auto h = YGNodeLayoutGetHeight(l);
  const auto w = YGNodeLayoutGetWidth(l);
  const auto rect = SkRect::MakeXYWH(x, y, w, h);

  auto paint = style.mColor->GetPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);

  const auto textHeight = h - metrics.fDescent;
  const auto textY = y + textHeight;
  canvas->drawString(mText.c_str(), x, textY, mFont, paint);
}

WidgetStyles Label::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mBase = {
      .mColor = SystemTheme::SystemForegroundColor,
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