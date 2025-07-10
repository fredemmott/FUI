// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchThumb final : public Widget {
 public:
  ToggleSwitchThumb(std::size_t id);

  [[nodiscard]]
  bool IsOn() const noexcept {
    return mIsOn;
  }

  void SetIsOn(bool) noexcept;

 private:
  void UpdateStyles();
  bool mIsOn {false};
  Widget* mInner {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
