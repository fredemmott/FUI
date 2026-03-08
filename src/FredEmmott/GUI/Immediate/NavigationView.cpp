// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationView.hpp"

#include <stack>

#include "FredEmmott/GUI/Widgets/NavigationView.hpp"
#include "FredEmmott/GUI/Widgets/NavigationViewItem.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"

namespace FredEmmott::GUI::Immediate {

namespace {
using Widgets::NavigationView;
using Widgets::NavigationViewItem;
using namespace immediate_detail;

struct NavigationViewState : Widgets::Context {
  // Default to first item
  std::size_t mSelectedItemIndex {};
  NavigationViewItem* mSelectedItem {};
};

struct NavigationViewStackEntry {
  NavigationView* mNavigationView {};
  NavigationViewState* mContext {};
  std::size_t mNextItemIndex {};
};

thread_local std::stack<NavigationViewStackEntry> tNavigationViewStack {};

void UpdateSelectedItem() {
  using namespace Widgets;
  const auto nav = tNavigationViewStack.top().mNavigationView;
  auto& [selectedIndex, selectedItem] = *tNavigationViewStack.top().mContext;

  if (!selectedItem) {
    FUI_ASSERT(!selectedIndex);
    return;
  }

  const auto mainItems = nav->GetItemsRoot()->GetLogicalChildren();
  const auto footerItems = nav->GetItemsRoot()->GetLogicalChildren();

  // TODO (C++23): use std::views::concat when it's available in MSVC
  const auto items = std::array {
    std::views::all(mainItems),
    std::views::all(footerItems),
  }
    | std::views::join
    | std::views::transform([](Widget* p) { return dynamic_cast<NavigationViewItem*>(p); })
    | std::views::filter([](NavigationViewItem* p) { return p != nullptr; })
    | std::ranges::to<std::vector>();

  if (items.empty()) {
    selectedIndex = 0;
    selectedItem = nullptr;
    return;
  }

  const auto consistency = felly::scope_exit {[selectedItem, &items] {
    for (auto&& item: items) {
      if (item != selectedItem) {
        item->SetIsChecked(false);
      }
    }
  }};

  if (const auto it = std::ranges::find_if(
        items, [](NavigationViewItem* p) { return p->ConsumeWasActivated(); });
      it != items.end()) {
    selectedIndex = std::distance(items.begin(), it);
    selectedItem = *it;
    return;
  }

  if (const auto it = std::ranges::find(items, selectedItem);
      it != items.end()) {
    selectedIndex = std::distance(items.begin(), it);
    return;
  }

  selectedIndex = std::min(selectedIndex, items.size() - 1);
  selectedItem = items[selectedIndex];
}

}// namespace

NavigationViewResult BeginNavigationView(const ID id) {
  const auto w = ChildlessWidget<NavigationView>(id);
  tNavigationViewStack.emplace(w, w->GetOrCreateContext<NavigationViewState>());
  PushParentOverride(w->GetItemsRoot());

  UpdateSelectedItem();

  return {w};
}

void EndNavigationView() {
  auto w = tNavigationViewStack.top().mNavigationView;
  std::ignore = w;
  PopParentOverride(tNavigationViewStack.top().mNavigationView->GetItemsRoot());
  tNavigationViewStack.pop();
  // **NOT** calling EndWidget<NavigationView> because we created it as a
  // 'ChildlessWidget'
}

NavigationViewItemResult BeginNavigationViewItem(
  const std::string_view icon,
  const std::string_view label,
  const std::string_view headerText,
  const ID id) {
  FUI_ASSERT(!tNavigationViewStack.empty());
  const auto w
    = ChildlessWidget<NavigationViewItem>(id)->SetIcon(icon)->SetText(label);

  auto& top = tNavigationViewStack.top();
  const auto idx = top.mNextItemIndex++;
  auto& ctx = *top.mContext;
  if (idx != ctx.mSelectedItemIndex) {
    w->SetIsChecked(false);
    return {w, false};
  }
  ctx.mSelectedItem = w;
  w->SetIsChecked(true);
  std::ignore = w->ConsumeWasActivated();

  top.mNavigationView->SetHeaderText(headerText);
  PushParentOverride(top.mNavigationView->GetContentRoot());
  return {w, true};
}

void EndNavigationViewItem() {
  FUI_ASSERT(!tNavigationViewStack.empty());
  PopParentOverride(
    tNavigationViewStack.top().mNavigationView->GetContentRoot());
}

}// namespace FredEmmott::GUI::Immediate