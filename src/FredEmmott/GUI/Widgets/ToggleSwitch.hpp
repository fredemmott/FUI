// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitch final : public Widget, public IToggleable {
 public:
  explicit ToggleSwitch(std::size_t id);

  [[nodiscard]]
  bool IsOn() const noexcept;
  void SetIsOn(bool) noexcept;

  bool mChanged {false};

  void Toggle() override;

 protected:
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

  Widget* GetFosterParent() const noexcept override {
    return mFosterParent;
  }

  EventHandlerResult OnClick(const MouseEvent& event) override;

 private:
  Widget* mFosterParent {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
