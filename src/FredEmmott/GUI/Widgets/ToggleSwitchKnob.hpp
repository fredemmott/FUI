// SPDX-License-Identifier: MIT
#pragma once

#include "ToggleSwitchThumb.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchKnob final : public Widget {
 public:
  ToggleSwitchKnob(std::size_t id);

  [[nodiscard]]
  bool IsOn() const noexcept;
  void SetIsOn(bool) noexcept;

 protected:
  WidgetList GetDirectChildren() const noexcept override;

 private:
  void UpdateStyles();
  unique_ptr<ToggleSwitchThumb> mThumb {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
