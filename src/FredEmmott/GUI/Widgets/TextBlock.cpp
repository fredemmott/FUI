// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "TextBlock.hpp"

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/renderer_detail.hpp>

#ifdef FUI_ENABLE_DIRECT2D
#include "FredEmmott/GUI/Direct2DRenderer.hpp"
#endif

#ifdef FUI_ENABLE_SKIA
#include <skia/core/SkFont.h>
#include <skia/core/SkFontMgr.h>
#include <skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <skia/modules/skunicode/include/SkUnicode_icu.h>
#include <skia/ports/SkFontMgr_empty.h>

#include <FredEmmott/GUI/SkiaRenderer.hpp>
#endif

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::renderer_detail;

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
  if (GetRenderAPI() == RenderAPI::Skia) {
    this->UpdateSkiaParagraph();
  }
#endif
#ifdef FUI_ENABLE_DIRECT2D
  if (GetRenderAPI() == RenderAPI::Direct2D) {
    this->UpdateDirectWriteTextLayout();
  }
#endif
  if constexpr (Config::Debug) {
    __debugbreak();
  }
}

void TextBlock::PaintOwnContent(
  Renderer* renderer,
  const Rect& outerRect,
  const Style& style) const {
  FUI_ASSERT(
    style.mFont == mFont,
    "Stylesheet font does not match mFont; computed style not updated");

  const auto yoga = this->GetLayoutNode();
  const Rect rect = outerRect.WithInset(
    YGNodeLayoutGetPadding(yoga, YGEdgeLeft)
      + YGNodeLayoutGetBorder(yoga, YGEdgeLeft),
    YGNodeLayoutGetPadding(yoga, YGEdgeTop)
      + YGNodeLayoutGetBorder(yoga, YGEdgeTop),
    YGNodeLayoutGetPadding(yoga, YGEdgeRight)
      + YGNodeLayoutGetBorder(yoga, YGEdgeRight),
    YGNodeLayoutGetPadding(yoga, YGEdgeBottom)
      + YGNodeLayoutGetBorder(yoga, YGEdgeBottom));

#ifdef FUI_ENABLE_SKIA
  if (auto canvas = skia_canvas_cast(renderer)) {
    this->PaintOwnContent(renderer, canvas, rect, style);
    return;
  }
#endif
#ifdef FUI_ENABLE_DIRECT2D
  if (auto d2d = direct2d_device_context_cast(renderer)) {
    this->PaintOwnContent(renderer, d2d, rect, style);
    return;
  }
#endif
  FUI_ALWAYS_ASSERT(false, "TextBlock currently requires Skia")
}

Widget::ComputedStyleFlags TextBlock::OnComputedStyleChange(
  const Style& style,
  StateFlags) {
  if (mFont != style.mFont) {
    mFont = style.mFont.value();
  }

#ifdef FUI_ENABLE_SKIA
  if (GetRenderAPI() == RenderAPI::Skia) {
    this->UpdateSkiaParagraph();
  }
#endif
#ifdef FUI_ENABLE_DIRECT2D
  if (GetRenderAPI() == RenderAPI::Direct2D) {
    this->UpdateDirectWriteTextLayout();
  }
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
#ifdef FUI_ENABLE_DIRECT2D
  if (self->mDirectWriteTextLayout) {
    return self->MeasureWithDirectWrite(width, widthMode, height, heightMode);
  }
#endif
  if constexpr (Config::Debug) {
    __debugbreak();
  }
  return {YGUndefined, YGUndefined};
}

}// namespace FredEmmott::GUI::Widgets