// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitch final : public Widget, public IToggleable {
 public:
  explicit ToggleSwitch(Window*);
  ~ToggleSwitch() override;

  [[nodiscard]]
  bool IsOn() const noexcept;
  void SetIsOn(bool) noexcept;

  void Toggle() override;

  [[nodiscard]]
  bool ConsumeWasChanged() noexcept {
    return std::exchange(mWasChanged, false);
  }

 protected:
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

  EventHandlerResult OnClick(const MouseEvent& event) override;

 private:
  Widget* mFosterParent {nullptr};
  bool mWasChanged {false};
};

}// namespace FredEmmott::GUI::Widgets
