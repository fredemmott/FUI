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

  ISelectionContainer* GetSelectionContainer() const noexcept override {
    if (!mSelectionContainer) {
      mSelectionContainer
        = dynamic_cast<ISelectionContainer*>(this->GetLogicalParent());
      FUI_ASSERT(mSelectionContainer);
    }
    return mSelectionContainer;
  }

  [[nodiscard]]
  bool ConsumeWasChanged() noexcept {
    return std::exchange(mWasChanged, false);
  }

 protected:
  Widget* GetStructuralParentForLogicalChildren() noexcept override;
  ComputedStyleFlags OnComputedStyleChange(const Style& style, StateFlags state)
    override;
  [[nodiscard]] EventHandlerResult OnClick(const MouseEvent&) override;

 private:
  mutable ISelectionContainer* mSelectionContainer {};
  Widget* mFosterParent {};
  bool mWasChanged {false};
};

}// namespace FredEmmott::GUI::Widgets