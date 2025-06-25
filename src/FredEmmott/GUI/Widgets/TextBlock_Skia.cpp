// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <Windows.h>
#include <skia/core/SkFont.h>
#include <skia/core/SkFontMgr.h>
#include <skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <skia/modules/skunicode/include/SkUnicode_icu.h>
#include <skia/ports/SkFontMgr_empty.h>

#include <FredEmmott/GUI/SkiaRenderer.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/utility/lazy_init.hpp>

#include "FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp"
#include "TextBlock.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

YGSize TextBlock::MeasureWithSkia(
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  if (widthMode == YGMeasureModeUndefined) {
    return {YGUndefined, YGUndefined};
  }

  mSkiaParagraph->layout(width);
  mMeasuredHeight = mSkiaParagraph->getHeight();

  return {width, mMeasuredHeight};
}

void TextBlock::UpdateSkiaParagraph() {
  static const auto SkiaICU = SkUnicodes::ICU::Make();
  static const auto FontCollection
    = sk_make_sp<skia::textlayout::FontCollection>();
  if (FontCollection->getFontManagersCount() == 0) {
    FontCollection->setDefaultFontManager(SystemFont::GetFontManager());
  }
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
    paragraphStyle, FontCollection, SkiaICU);
  builder->addText(mText.data(), mText.size());
  mSkiaParagraph = builder->Build();

  YGNodeMarkDirty(this->GetLayoutNode());
}

void TextBlock::PaintOwnContent(
  SkCanvas* canvas,
  const Rect& rect,
  const Style& style) const {
  FUI_ASSERT(
    style.mFont == mFont,
    "Stylesheet font does not match mFont; computed style not updated");

  auto paint = style.mColor->GetSkiaPaint(rect);
  paint.setStyle(SkPaint::Style::kFill_Style);
  mSkiaParagraph->updateForegroundPaint(0, mText.size(), paint);
  mSkiaParagraph->paint(canvas, rect.GetLeft(), rect.GetTop());
}

}// namespace FredEmmott::GUI::Widgets