// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Label final : public Widget {
 public:
  Label(std::size_t id);

  std::string_view GetText() const noexcept {
    return mText;
  }
  void SetText(std::string_view);

 protected:
  void PaintOwnContent(SkCanvas*, const Style& style) const override;
  WidgetStyles GetDefaultStyles() const override;
  std::optional<Style> GetInstanceStyles(const Style& base) const override;

 private:
  std::string mText;
  Font mFont;

  static YGSize Measure(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode);
};

}// namespace FredEmmott::GUI::Widgets
