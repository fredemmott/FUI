// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchThumb final : public Widget {
 public:
  explicit ToggleSwitchThumb(id_type id);

 private:
  Widget* mInner {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
