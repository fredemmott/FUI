// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Button.hpp"
#include "Focusable.hpp"

namespace FredEmmott::GUI::Widgets {

class ComboBoxItemButton : public Button, public ISelectionItem {
 public:
  ComboBoxItemButton(id_type id);
  ~ComboBoxItemButton() override;

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  bool IsSelected() const noexcept override {
    return IsChecked();
  }

  void Select() override {
    this->Invoke();
  }
};

}// namespace FredEmmott::GUI::Widgets