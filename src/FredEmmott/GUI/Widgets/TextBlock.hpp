// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>

#ifdef FUI_ENABLE_SKIA
#include <skia/modules/skparagraph/include/Paragraph.h>
#endif

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class TextBlock final : public Widget {
 public:
  explicit TextBlock(std::size_t id);

  void SetText(std::string_view);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style& style)
    const override;
  Style GetBuiltInStyles() const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& base, StateFlags state)
    override;

 private:
#ifdef FUI_ENABLE_SKIA
  std::unique_ptr<skia::textlayout::Paragraph> mSkiaParagraph;
#endif
  std::string mText;
  Font mFont;
  float mMeasuredHeight {};

#ifdef FUI_ENABLE_SKIA
  void UpdateSkiaParagraph();
  void PaintOwnContent(SkCanvas*, const Rect&, const Style&) const;
  YGSize MeasureWithSkia(
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
#endif

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};

}// namespace FredEmmott::GUI::Widgets
