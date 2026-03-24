// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Button.hpp"
#include "Focusable.hpp"

namespace FredEmmott::GUI::Widgets {

class ComboBoxItemButton : public Button, public ISelectionItem {
 public:
  ComboBoxItemButton();
  ~ComboBoxItemButton() override;

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  bool IsSelected() const noexcept override {
    return IsChecked();
  }

  void Select() override {
    this->Invoke();
  }

  ISelectionContainer* GetSelectionContainer() const noexcept override {
    if (!mSelectionContainer) {
      mSelectionContainer
        = dynamic_cast<ISelectionContainer*>(this->GetLogicalParent());
      FUI_ASSERT(mSelectionContainer);
    }
    return mSelectionContainer;
  }

  bool ConsumeWasSelected() noexcept override {
    return ConsumeWasActivated();
  }

 private:
  mutable ISelectionContainer* mSelectionContainer {};
};

}// namespace FredEmmott::GUI::Widgets