// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Label.hpp"

#include <FredEmmott/utility/lazy_init.hpp>

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

namespace {
const lazy_init<Style> DefaultLabelStyle = [] {
  return Style {
    .mColor = SystemColor::Foreground,
    .mFont = SystemFont::Body,
  };
};
}// namespace

void Label::SetLayoutConstraints() {
  const auto style = this->GetStyle();

  const auto width
    = (*style.mFont)
        ->measureText(mText.data(), mText.size(), SkTextEncoding::kUTF8);
  const auto height = style.mFont->GetHeightInPixels();

  const auto l = this->GetLayoutNode();
  YGNodeStyleSetWidth(l, width);
  YGNodeStyleSetHeight(l, height);
}

Style Label::GetStyle() const noexcept {
  return DefaultLabelStyle.Get()
    + (IsHovered() ? mOptions.mStyle : mOptions.mHoverStyle);
}

Label::Label(std::size_t id, const Options& options)
  : Widget(id), mOptions(options) {
}

void Label::SetText(std::string_view text) {
  mText = std::string {text};
  this->SetLayoutConstraints();
}

void Label::PaintOwnContent(SkCanvas* canvas, const WidgetStyles&) const {
  const auto l = this->GetLayoutNode();
  const auto style = this->GetStyle();
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

}// namespace FredEmmott::GUI::Widgets