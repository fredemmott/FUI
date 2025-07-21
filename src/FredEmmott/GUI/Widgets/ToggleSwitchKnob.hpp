// SPDX-License-Identifier: MIT
#pragma once

#include "ToggleSwitchThumb.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchKnob final : public Widget {
 public:
  ToggleSwitchKnob(std::size_t id);

 protected:
  WidgetList GetDirectChildren() const noexcept override;

 private:
  unique_ptr<ToggleSwitchThumb> mThumb {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
