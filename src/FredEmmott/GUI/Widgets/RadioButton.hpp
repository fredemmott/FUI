// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class RadioButton final : public Widget, public ISelectionItem {
 public:
  explicit RadioButton(id_type id);
  ~RadioButton() override;

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  [[nodiscard]]
  bool IsSelected() const noexcept override;
  void Select() override;

  [[nodiscard]]
  bool ConsumeWasChanged() noexcept {
    return std::exchange(mWasChanged, false);
  }

 protected:
  Widget* GetFosterParent() const noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
  [[nodiscard]] EventHandlerResult OnClick(const MouseEvent&) override;

 private:
  Widget* mFosterParent {};
  bool mWasChanged {false};
};

}// namespace FredEmmott::GUI::Widgets