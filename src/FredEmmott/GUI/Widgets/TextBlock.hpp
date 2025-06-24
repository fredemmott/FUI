// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/modules/skparagraph/include/Paragraph.h>

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class TextBlock final : public Widget {
 public:
  TextBlock(std::size_t id);
  void UpdateParagraph();

  std::string_view GetText() const noexcept {
    return mText;
  }
  void SetText(std::string_view);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style& style)
    const override;
  Style GetBuiltInStyles() const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& base, StateFlags state)
    override;

 private:
  std::unique_ptr<skia::textlayout::Paragraph> mParagraph;
  std::string mText;
  Font mFont;
  float mMeasuredHeight {};

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};

}// namespace FredEmmott::GUI::Widgets
