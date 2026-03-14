// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Focusable.hpp"
#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {
class NavigationViewItem;

class Label;

class NavigationView final : public Widget, public ISelectionContainer {
 public:
  explicit NavigationView(id_type id);
  ~NavigationView() override;

  [[nodiscard]]
  Widget* GetItemsRoot() const noexcept {
    return mItemsRoot;
  }

  [[nodiscard]]
  Widget* GetFooterItemsRoot() const noexcept {
    return mFooterItemsRoot;
  }

  [[nodiscard]]
  Widget* GetContentRoot() const noexcept {
    return mContentInner;
  }

  void SetHeaderText(std::string_view);

  [[nodiscard]] std::vector<ISelectionItem*> GetSelectionItems()
    const noexcept override;

 private:
  // Contains menu/NavigationViewItems
  Widget* mPane {};
  Widget* mPaneHeader {};
  Widget* mItemsRoot {};
  Widget* mFooterItemsRoot {};

  // Has the rounded corner, contains header and mContentInner
  Widget* mContentOuter {};
  Label* mContentHeader {};
  // No rounded corner, just a container for child elements
  Widget* mContentInner {};
};

}// namespace FredEmmott::GUI::Widgets