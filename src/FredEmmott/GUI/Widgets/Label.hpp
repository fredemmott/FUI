// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Label final : public Widget {
 public:
  struct Options {
    Style mStyle;
    Style mHoverStyle;
  };

  Label(std::size_t id, const Options&);
  void PaintOwnContent(SkCanvas*, const WidgetStyles&) const override;

  std::string_view GetText() const noexcept {
    return mText;
  }
  void SetText(std::string_view);

 private:
  Options mOptions;
  std::string mText;

  void SetLayoutConstraints();
  Style GetStyle() const noexcept;
};

}// namespace FredEmmott::GUI::Widgets
