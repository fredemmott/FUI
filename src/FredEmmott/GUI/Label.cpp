// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

namespace FredEmmott::GUI {

void Label::SetLayoutConstraints() {
  const auto width = mOptions.mFont->measureText(
    mText.data(), mText.size(), SkTextEncoding::kUTF8);
  const auto height = mOptions.mFont.GetPixelHeight();

  const auto l = this->GetLayoutNode();
  YGNodeStyleSetWidth(l, width);
  YGNodeStyleSetHeight(l, height);
}

void Label::Paint(SkCanvas* canvas) const {
  const auto l = this->GetLayoutNode();

  SkPaint paint;
  paint.setStyle(SkPaint::kFill_Style);
  paint.setColor(mOptions.mColor);

  const auto x = YGNodeLayoutGetLeft(l);
  const auto top = YGNodeLayoutGetTop(l);
  const auto height = YGNodeLayoutGetHeight(l);
  const auto y = top + height;

  canvas->drawString(
    mText.c_str(),
    x, y,
    mOptions.mFont.Get(),
    paint);
}

}// namespace FredEmmott::GUI