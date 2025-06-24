// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBlock.hpp"

#include <Windows.h>
#include <skia/core/SkFont.h>
#include <skia/core/SkFontMgr.h>
#include <skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <skia/modules/skunicode/include/SkUnicode_icu.h>
#include <skia/ports/SkFontMgr_empty.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/utility/lazy_init.hpp>

#include "FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp"
#include "FredEmmott/GUI/SkiaRenderer.hpp"

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
  const auto font = SkFont {mFont};
  font.getTypeface()->getFamilyName(&familyName);
  TextStyle textStyle;
  textStyle.setFontFamilies({familyName});
  textStyle.setFontSize(font.getSize());
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
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
#ifndef NDEBUG
  if (style.mFont != mFont) [[unlikely]] {
    throw std::logic_error(
      "Stylesheet font does not match mFont; computed style not updated");
  }
#endif
  auto canvas = skia_canvas_cast(renderer);
  FUI_ASSERT(canvas, "TextBlock currently only supports Skia");

  auto paint = style.mColor->GetSkiaPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mParagraph->updateForegroundPaint(0, mText.size(), paint);
  mParagraph->paint(canvas, rect.GetLeft(), rect.GetTop());
}

Style TextBlock::GetBuiltInStyles() const {
  static const Style ret {
    .mFlexGrow = 0,
    .mFlexShrink = 1,
  };
  return ret;
}

Widget::ComputedStyleFlags TextBlock::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  if (mFont != style.mFont) {
    mFont = style.mFont.value();
  }
  this->UpdateParagraph();

  return ComputedStyleFlags::Empty;
}

YGSize TextBlock::Measure(
  YGNodeConstRef node,
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto self = static_cast<TextBlock*>(FromYogaNode(node));

  if (widthMode == YGMeasureModeUndefined) {
    return {YGUndefined, YGUndefined};
  }

  self->mParagraph->layout(width);
  self->mMeasuredHeight = self->mParagraph->getHeight();

  return {width, self->mMeasuredHeight};
}

}// namespace FredEmmott::GUI::Widgets