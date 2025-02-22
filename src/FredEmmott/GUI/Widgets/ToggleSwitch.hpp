// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/ActivatedFlag.hpp>

#include "ToggleSwitchKnob.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitch final : public Widget {
 public:
  ToggleSwitch(std::size_t id);

  [[nodiscard]]
  bool IsOn() const noexcept;
  void SetIsOn(bool) noexcept;

  ActivatedFlag mChanged;

 protected:
  Style GetBuiltInStyles() const override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
  Widget* GetFosterParent() const noexcept override {
    return mFosterParent.get();
  }

  EventHandlerResult OnClick(const MouseEvent& event) override;

 private:
  unique_ptr<ToggleSwitchKnob> mKnob;
  // Container for user-supplied children
  unique_ptr<Widget> mFosterParent;
  WidgetList GetDirectChildren() const noexcept override;
};

}// namespace FredEmmott::GUI::Widgets
