// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

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
class NavigationViewItem final : public Widget {
 public:
  explicit NavigationViewItem(id_type id);
  ~NavigationViewItem() override;

  [[nodiscard]]
  bool ConsumeWasActivated() noexcept {
    return std::exchange(mWasActivated, false);
  }

  using Widget::IsChecked;
  using Widget::SetIsChecked;

  auto* SetIcon(const std::string_view text) {
    mIcon->SetText(text);
    return this;
  }

  auto* SetText(const std::string_view text) {
    mText->SetText(text);
    return this;
  }

 protected:
  EventHandlerResult OnClick(const MouseEvent&) override;
  bool mWasActivated = false;

  Label* mIcon {};
  Label* mText {};
};

}// namespace FredEmmott::GUI::Widgets
