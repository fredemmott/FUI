// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// See skia_paragraph.hpp for what this is for and why.

#include <FredEmmott/GUI/detail/skia_paragraph.hpp>

#include <skia/core/SkString.h>
#include <skia/core/SkTypeface.h>
#include <skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <skia/modules/skunicode/include/SkUnicode_icu.h>

#include <FredEmmott/GUI/SystemFont.hpp>

namespace FredEmmott::GUI::skia_paragraph_detail {

std::unique_ptr<skia::textlayout::Paragraph> BuildSingleStyleParagraph(
  const SkFont& font,
  const std::string_view text) {
  // Static singletons — mirrors TextBlock_Skia.cpp; rebuilding per call is
  // wasted work, both are program-lifetime config.
  static const auto SkiaICU = SkUnicodes::ICU::Make();
  static const auto FontCollection = []() {
    auto fc = sk_make_sp<skia::textlayout::FontCollection>();
    fc->setDefaultFontManager(SystemFont::GetFontManager());
    return fc;
  }();
  using namespace skia::textlayout;

  SkString familyName;
  if (auto* tf = font.getTypeface()) {
    tf->getFamilyName(&familyName);
  }
  TextStyle textStyle;
  textStyle.setFontFamilies({familyName});
  textStyle.setFontSize(font.getSize());
  ParagraphStyle paragraphStyle;
  paragraphStyle.setTextStyle(textStyle);

  auto builder
    = ParagraphBuilder::make(paragraphStyle, FontCollection, SkiaICU);
  builder->addText(text.data(), text.size());
  return builder->Build();
}

}// namespace FredEmmott::GUI::skia_paragraph_detail
