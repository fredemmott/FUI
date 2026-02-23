// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class CheckBox final : public Widget, public IToggleable {
 public:
  explicit CheckBox(std::size_t id);

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  void Toggle() override;

  [[nodiscard]]
  bool ConsumeWasChanged() noexcept {
    return std::exchange(mWasChanged, false);
  }

 protected:
  EventHandlerResult OnClick(const MouseEvent& event) override;
  Widget* GetFosterParent() const noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  Widget* mFosterParent {nullptr};
  bool mWasChanged {false};
};

}// namespace FredEmmott::GUI::Widgets
