// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
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
class NavigationViewItem final : public Widget, public ISelectionItem {
 public:
  explicit NavigationViewItem(id_type id);
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
  EventHandlerResult OnClick(const MouseEvent&) override;
  bool mWasSelected = false;

  Widget* mIconHolder {};
  Label* mIcon {};
  Label* mText {};
};

}// namespace FredEmmott::GUI::Widgets
