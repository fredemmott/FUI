// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchThumb final : public Widget {
 public:
  explicit ToggleSwitchThumb(std::size_t id);

 private:
  void UpdateStyles();
  Widget* mInner {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
