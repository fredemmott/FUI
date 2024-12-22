// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "../Color.hpp"
#include "../WidgetColor.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Card final : public Widget {
 public:
  struct Options {
    Color mBackgroundColor {WidgetColor::CardBackgroundFillDefault};
  };

  Card(std::size_t id, const Options&, Widget* child);
  void Paint(SkCanvas* canvas) const override;

 private:
  Options mOptions;
  Widget* mChild {nullptr};
};

}// namespace FredEmmott::GUI::Widgets