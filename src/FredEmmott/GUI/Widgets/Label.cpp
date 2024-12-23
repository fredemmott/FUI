// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/utility/lazy_init.hpp>

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

void Label::SetLayoutConstraints() {
  // FIXME: need to pass styles through
  const auto style = this->GetDefaultStyles().mDefault;

  const auto width
    = (*style.mFont)
        ->measureText(mText.data(), mText.size(), SkTextEncoding::kUTF8);
  const auto height = style.mFont->GetHeightInPixels();

  const auto l = this->GetLayoutNode();
  YGNodeStyleSetWidth(l, width);
  YGNodeStyleSetHeight(l, height);
}

Label::Label(std::size_t id) : Widget(id) {
}

void Label::SetText(std::string_view text) {
  if (text == mText) {
    return;
  }
  mText = std::string {text};
  this->SetLayoutConstraints();
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

}// namespace FredEmmott::GUI::Widgets