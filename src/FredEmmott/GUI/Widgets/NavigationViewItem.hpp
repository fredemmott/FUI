// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "FredEmmott/GUI/detail/SelectionPill.hpp"
#include "Label.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

/** The 'button' in the navigation pane of a navigation view.
 *
 * On click, this is activated and checked.
 * It is only un-checked by explicit calls to `SetIsChecked()`
 *
 * This means they function roughly the same as radio buttons.
 */
class NavigationViewItem : public Widget, public ISelectionItem {
 public:
  NavigationViewItem();
  ~NavigationViewItem() override;

  auto* SetIcon(const std::string_view text) {
    mIcon->SetText(text);
    return this;
  }

  auto* SetText(const std::string_view text) {
    mText->SetText(text);
    return this;
  }

  ///// ISelectionItem /////
  void Select() override;
  bool IsSelected() const noexcept override {
    return IsChecked();
  }
  ISelectionContainer* GetSelectionContainer() const noexcept override;

  [[nodiscard]]
  bool ConsumeWasSelected() noexcept override {
    return std::exchange(mWasSelected, false);
  }

 protected:
  void SetIconRotation(const float degrees) noexcept {
    mIcon->SetMutableStyles(Style().Rotate(degrees));
  }

  void Tick(const std::chrono::steady_clock::time_point& now) override;
  EventHandlerResult OnClick(const MouseEvent&) override;
  FrameRateRequirement GetFrameRateRequirement() const noexcept override;
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  bool mWasSelected = false;

  Widget* mIconHolder {};
  Label* mIcon {};
  Label* mText {};
  detail::SelectionPill mSelectionPill {
    detail::SelectionPill::State::NotSelected};
};

}// namespace FredEmmott::GUI::Widgets
