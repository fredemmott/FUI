// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchKnob final : public Widget {
 public:
  ToggleSwitchKnob(std::size_t id);

  ActivatedFlag mChanged;
  bool mIsOn {false};

 protected:
  WidgetStyles GetDefaultStyles() const override;
  EventHandlerResult OnClick(MouseEvent* e) override;
  ComputedStyleFlags OnComputedStyleChange(const Style& base) override;
};

}// namespace FredEmmott::GUI::Widgets
