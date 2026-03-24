// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Label final : public Widget {
 public:
  explicit Label(Window*);
  Label(
    Window*,
    StyleClass primaryClass,
    const ImmutableStyle&,
    const StyleClasses& = {});
  ~Label() override;

  [[nodiscard]]
  std::string_view GetText() const noexcept {
    return mText;
  }
  Label* SetText(std::string_view);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style& style)
    const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& base, StateFlags state)
    override;

 private:
  std::string mText;
  Font mFont;

  std::optional<YGSize> mCachedMeasurement;

  static YGSize Measure(
    const YGNode* node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};

}// namespace FredEmmott::GUI::Widgets
