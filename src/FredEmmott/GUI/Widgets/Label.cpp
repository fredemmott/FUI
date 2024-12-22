// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

void Label::SetLayoutConstraints() {
  const auto width = mOptions.mFont->measureText(
    mText.data(), mText.size(), SkTextEncoding::kUTF8);
  const auto height = mOptions.mFont.GetHeightInPixels();

  const auto l = this->GetLayoutNode();
  YGNodeStyleSetWidth(l, width);
  YGNodeStyleSetHeight(l, height);
}

Label::Label(std::size_t id, const Options& options, std::string_view text)
  : Widget(id), mOptions(options), mText(text) {
  this->SetLayoutConstraints();
}

void Label::Paint(SkCanvas* canvas) const {
  const auto l = this->GetLayoutNode();

  SkPaint paint;
  paint.setStyle(SkPaint::kFill_Style);
  paint.setColor(mOptions.mColor);

  SkFontMetrics metrics {};
  mOptions.mFont.GetMetricsInPixels(&metrics);

  const auto x = YGNodeLayoutGetLeft(l);
  const auto top = YGNodeLayoutGetTop(l);
  const auto height = YGNodeLayoutGetHeight(l) - metrics.fDescent;
  const auto y = top + height;

  canvas->drawString(mText.c_str(), x, y, mOptions.mFont, paint);
}

}// namespace FredEmmott::GUI::Widgets