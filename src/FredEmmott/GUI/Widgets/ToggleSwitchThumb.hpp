// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchThumb final : public Widget {
 public:
  ToggleSwitchThumb(std::size_t id);
  bool mIsOn {false};

 protected:
  WidgetStyles GetDefaultStyles() const override;
};

}// namespace FredEmmott::GUI::Widgets
