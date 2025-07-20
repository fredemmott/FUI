// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Button : public Widget {
 public:
  explicit Button(std::size_t id);

  bool mClicked {false};

 protected:
  EventHandlerResult OnClick(const MouseEvent& e) override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
};

}// namespace FredEmmott::GUI::Widgets
