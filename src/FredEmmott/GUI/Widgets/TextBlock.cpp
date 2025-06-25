// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBlock.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/utility/lazy_init.hpp>

#include "FredEmmott/GUI/Immediate/EnqueueAdditionalFrame.hpp"

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFont.h>
#include <skia/core/SkFontMgr.h>
#include <skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <skia/modules/skunicode/include/SkUnicode_icu.h>
#include <skia/ports/SkFontMgr_empty.h>

#include <FredEmmott/GUI/SkiaRenderer.hpp>
#endif

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

TextBlock::TextBlock(std::size_t id) : Widget(id) {
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &TextBlock::Measure);
  YGNodeSetNodeType(this->GetLayoutNode(), YGNodeTypeText);
}

void TextBlock::SetText(const std::string_view text) {
  if (text == mText) {
    return;
  }
  mText = std::string {text};

  if (!mFont) {
    return;
  }

#ifdef FUI_ENABLE_SKIA
  this->UpdateSkiaParagraph();
#endif
}

void TextBlock::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
#ifdef FUI_ENABLE_SKIA
  if (auto* canvas = skia_canvas_cast(renderer)) {
    this->PaintOwnContent(canvas, rect, style);
    return;
  }
#endif
  FUI_ALWAYS_ASSERT(false, "TextBlock currently requires Skia")
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

#ifdef FUI_ENABLE_SKIA
  this->UpdateSkiaParagraph();
#endif

  return ComputedStyleFlags::Empty;
}

YGSize TextBlock::Measure(
  YGNodeConstRef node,
  float width,
  YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto self = static_cast<TextBlock*>(FromYogaNode(node));
#ifdef FUI_ENABLE_SKIA
  if (self->mSkiaParagraph) {
    return self->MeasureWithSkia(width, widthMode, height, heightMode);
  }
#endif
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  return {YGUndefined, YGUndefined};
}

}// namespace FredEmmott::GUI::Widgets