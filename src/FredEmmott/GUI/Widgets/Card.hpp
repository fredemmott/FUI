// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "../Color.hpp"
#include "../WidgetColor.hpp"
#include "IHasSingleChild.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Card final : public Widget, public IHasSingleChild {
 public:
  struct Options {
    Color mBackgroundColor {WidgetColor::CardBackgroundFillDefault};
  };

  Card(std::size_t id, const Options&);
  void Paint(SkCanvas* canvas) const override;

  Widget* GetChild() const noexcept override;
  void SetChild(Widget* child) override;

 private:
  Options mOptions;
  Widget* mChild {nullptr};
};

}// namespace FredEmmott::GUI::Widgets