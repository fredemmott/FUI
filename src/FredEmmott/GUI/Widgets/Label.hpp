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

  std::string_view GetText() const noexcept {
    return mText;
  }
  void SetText(std::string_view);

 protected:
  void PaintOwnContent(SkCanvas*, const Style& style) const override;
  WidgetStyles GetDefaultStyles() const override;

 private:
  Options mOptions;
  std::string mText;

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI::Widgets
