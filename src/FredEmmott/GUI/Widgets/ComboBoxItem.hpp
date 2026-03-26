// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Button.hpp"
#include "Focusable.hpp"
#include "FredEmmott/GUI/detail/SelectionPill.hpp"

namespace FredEmmott::GUI::Widgets {

class ComboBoxItem : public Button, public ISelectionItem {
 public:
  explicit ComboBoxItem(Window*);
  ~ComboBoxItem() override;

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

  [[nodiscard]]
  bool ConsumeWasSelected() noexcept override {
    return ConsumeWasActivated();
  }

  void Tick(const std::chrono::steady_clock::time_point& now) override;

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  void OnMouseEnter(const MouseEvent&) override;
  void OnMouseLeave(const MouseEvent&) override;
  [[nodiscard]]
  FrameRateRequirement GetFrameRateRequirement() const noexcept override;

 private:
  using SelectionPill = detail::SelectionPill;
  mutable ISelectionContainer* mSelectionContainer {};
  SelectionPill mSelectionPill {SelectionPill::State::NotSelected};
};

}// namespace FredEmmott::GUI::Widgets