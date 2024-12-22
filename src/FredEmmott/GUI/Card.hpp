// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Color.hpp"
#include "Widget.hpp"
#include "WidgetColor.hpp"

namespace FredEmmott::GUI {

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

}// namespace FredEmmott::GUI