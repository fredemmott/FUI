// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>

#ifdef FUI_ENABLE_SKIA
#include <skia/modules/skparagraph/include/Paragraph.h>
#endif

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/utility/bitflag_enums.hpp>
#include <FredEmmott/utility/type_tag.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class TextBlock final : public Widget {
 public:
  explicit TextBlock(std::size_t id);

  void SetText(std::string_view);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style& style)
    const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& base, StateFlags state)
    override;

 private:
  enum class DirtyFlags {
    None = 0,
    Text = 1 << 0,
    Font = 1 << 1,
    // https://issues.skia.org/issues/389111535
    Skia_Color = 1 << 2,
  };
  friend consteval bool is_bitflag_enum(utility::type_tag_t<DirtyFlags>);
#ifdef FUI_ENABLE_SKIA
  std::unique_ptr<skia::textlayout::Paragraph> mSkiaParagraph;
#endif
#ifdef FUI_ENABLE_DIRECT2D
  wil::com_ptr<IDWriteTextLayout> mDirectWriteTextLayout;
#endif
  std::string mText;
  Font mFont;
  float mMeasuredHeight {};

  void UpdateTextLayout(DirtyFlags);
#ifdef FUI_ENABLE_SKIA
  std::optional<Style::PropertyTypes::Color_t> mSkiaColor;
  void UpdateSkiaParagraph();
  YGSize MeasureWithSkia(
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
  void PaintOwnContent(Renderer*, SkCanvas*, const Rect&, const Style&) const;
#endif
#ifdef FUI_ENABLE_DIRECT2D
  void UpdateDirectWriteTextLayout();
  YGSize MeasureWithDirectWrite(
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
  void PaintOwnContent(Renderer*, ID2D1RenderTarget*, const Rect&, const Style&)
    const;
#endif

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};

}// namespace FredEmmott::GUI::Widgets
