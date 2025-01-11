// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBlock.hpp"

#include <Windows.h>
#include <skia/core/SkFontMgr.h>
#include <skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <skia/modules/skunicode/include/SkUnicode_icu.h>
#include <skia/ports/SkFontMgr_empty.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/utility/lazy_init.hpp>

#include "FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

namespace {

const auto gUnicode = SkUnicodes::ICU::Make();
auto gFontCollection = sk_make_sp<skia::textlayout::FontCollection>();

}// namespace

TextBlock::TextBlock(std::size_t id) : Widget(id) {
  if (gFontCollection->getFontManagersCount() == 0) {
    gFontCollection->setDefaultFontManager(SystemFont::GetFontManager());
  }
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &TextBlock::Measure);
  YGNodeSetNodeType(this->GetLayoutNode(), YGNodeTypeText);
}

void TextBlock::UpdateParagraph() {
  using namespace skia::textlayout;

  SkString familyName;
  mFont->getTypeface()->getFamilyName(&familyName);
  TextStyle textStyle;
  textStyle.setFontFamilies({familyName});
  textStyle.setFontSize(mFont->getSize());
  ParagraphStyle paragraphStyle;
  paragraphStyle.setTextStyle(textStyle);
  auto builder = skia::textlayout::ParagraphBuilder::make(
    paragraphStyle, gFontCollection, gUnicode);
  builder->addText(mText.data(), mText.size());
  mParagraph = builder->Build();

  YGNodeMarkDirty(this->GetLayoutNode());
}
void TextBlock::SetText(std::string_view text) {
  if (text == mText) {
    return;
  }
  mText = std::string {text};

  this->UpdateParagraph();
}

void TextBlock::PaintOwnContent(
  SkCanvas* canvas,
  const SkRect& rect,
  const Style& style) const {
#ifndef NDEBUG
  if (style.mFont != mFont) [[unlikely]] {
    throw std::logic_error(
      "Stylesheet font does not match mFont; computed style not updated");
  }
#endif

  auto paint = style.mColor->GetPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mParagraph->updateForegroundPaint(0, mText.size(), paint);
  mParagraph->paint(canvas, rect.x(), rect.y());
}

WidgetStyles TextBlock::GetBuiltInStyles() const {
  static const WidgetStyles ret {
    .mBase = {
      .mColor = StaticTheme::TextFillColorPrimaryBrush,
      .mFlexGrow = 0,
      .mFlexShrink = 1,
      .mFont = SystemFont::Body,
    },
    .mDisabled = {
      .mColor = StaticTheme::TextFillColorDisabledBrush,
    },
  };
  return ret;
}

Widget::ComputedStyleFlags TextBlock::OnComputedStyleChange(
  const Style& style) {
  if (mFont != style.mFont) {
    mFont = style.mFont.value();
  }
  this->UpdateParagraph();

  return ComputedStyleFlags::Default;
}

YGSize TextBlock::Measure(
  YGNodeConstRef node,
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto self = static_cast<TextBlock*>(YGNodeGetContext(node));

  if (widthMode == YGMeasureModeUndefined) {
    return {YGUndefined, YGUndefined};
  }

  self->mParagraph->layout(width);
  self->mMeasuredHeight = self->mParagraph->getHeight();

  return {width, self->mMeasuredHeight};
}

}// namespace FredEmmott::GUI::Widgets