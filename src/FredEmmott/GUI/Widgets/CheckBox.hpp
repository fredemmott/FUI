// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Label.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class CheckBox final : public Widget {
 public:
  explicit CheckBox(std::size_t id);

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  bool mChanged {false};

 protected:
  EventHandlerResult OnClick(const MouseEvent& event) override;
  Widget* GetFosterParent() const noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;

 private:
  Widget* mFosterParent {nullptr};
};

}// namespace FredEmmott::GUI::Widgets
