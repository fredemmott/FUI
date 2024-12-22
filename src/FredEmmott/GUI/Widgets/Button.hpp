// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "../Color.hpp"
#include "IHasSingleChild.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button final : public Widget, public IHasSingleChild {
 public:
  struct Options {
    Color mFillColor {WidgetColor::ControlFillDefault};
    Color mBorderColor {WidgetColor::ControlElevationBorder};
  };

  Button(std::size_t id, const Options&);

  void Paint(SkCanvas* canvas) const override;
  Widget* GetChild() const noexcept override;
  void SetChild(Widget*) override;

 private:
  Options mOptions;
  Widget* mLabel {nullptr};

  void SetLayoutConstraints();
};

}// namespace FredEmmott::GUI::Widgets
