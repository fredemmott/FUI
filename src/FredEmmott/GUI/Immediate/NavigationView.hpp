// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/NavigationView.hpp>
#include <FredEmmott/GUI/Widgets/NavigationViewBackButton.hpp>
#include <FredEmmott/GUI/Widgets/NavigationViewItem.hpp>
#include <FredEmmott/GUI/detail/immediate/SelectionManager.hpp>
#include <FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp>
#include <stack>

#include "FredEmmott/GUI/Widgets/NavigationViewSettingsItem.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Result.hpp"
#include "selectable_key.hpp"

namespace FredEmmott::GUI::Immediate {

namespace immediate_detail {

template <selectable_key TKey>
struct NavigationViewContext : Widgets::Context {
  struct BackStack : std::stack<TKey> {
    void erase_if(auto&& predicate) {
      std::erase_if(this->c, std::forward<decltype(predicate)>(predicate));
    }
  };
  ~NavigationViewContext() override = default;

  BackStack mBackStack;
};

// Workaround for https://github.com/microsoft/STL/issues/6171
template <class T>
struct totally_ordered_msvc_workaround_t
  : std::bool_constant<std::totally_ordered<T>> {};
template <class... Ts>
struct totally_ordered_msvc_workaround_t<std::variant<Ts...>>
  : std::bool_constant<(std::totally_ordered<Ts> && ...)> {};

}// namespace immediate_detail

inline void EndNavigationView() {
  immediate_detail::PopParentOverride();
  // **NOT** calling EndWidget<NavigationView> because we created it as a
  // 'ChildlessWidget'
}

using NavigationViewResult = Result<&EndNavigationView, void>;

template <selectable_key T>
NavigationViewResult BeginNavigationView(
  T* selectedKey,
  const ID id = ID {std::source_location::current()}) {
  using namespace immediate_detail;
  const auto w = ChildlessWidget<Widgets::NavigationView>(id);

  auto& manager = SelectionManager<T>::Get(w);
  auto ctx = w->GetContext<NavigationViewContext<T>>();

  if (ctx && !ctx->mBackStack.empty()) {
    static constexpr bool Sortable
      = std::movable<T> && totally_ordered_msvc_workaround_t<T>::value;

    auto keys = manager.GetAllKeys();
    if constexpr (Sortable) {
      std::ranges::sort(keys);
    }

    ctx->mBackStack.erase_if([&keys](auto&& key) {
      if constexpr (Sortable) {
        return !std::ranges::binary_search(keys, key);
      } else {
        return !std::ranges::contains(keys, key);
      }
    });
    w->GetBackButton()->SetIsDirectlyDisabled(ctx->mBackStack.empty());
  }

  if (const auto back = w->GetBackButton(); back->ConsumeWasActivated()) {
    FUI_ASSERT(!ctx->mBackStack.empty());
    *selectedKey = ctx->mBackStack.top();
    ctx->mBackStack.pop();
    back->SetIsDirectlyDisabled(ctx->mBackStack.empty());
  }

  const auto oldKey = *selectedKey;
  manager.BeginContainer(selectedKey);
  if (*selectedKey != oldKey) {
    w->GetOrCreateContext<NavigationViewContext<T>>()->mBackStack.push(oldKey);
    w->GetBackButton()->SetIsDirectlyDisabled(false);
  }
  PushParentOverride(w->GetItemsRoot());
  return {w};
}

inline void EndNavigationViewItem() {
  immediate_detail::PopParentOverride();
}

using NavigationViewItemResult = Result<
  &EndNavigationViewItem,
  bool,
  immediate_detail::ToolTipResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin>;

template <selectable_key T>
NavigationViewItemResult BeginNavigationViewItem(
  const T key,
  const std::string_view icon,
  const std::string_view label,
  const std::string_view headerText,
  const ID id = ID {std::source_location::current()}) {
  using namespace immediate_detail;
  const auto w
    = immediate_detail::ChildlessWidget<Widgets::NavigationViewItem>(id)
        ->SetIcon(icon)
        ->SetText(label);

  const auto selectedThisFrame = SelectionManager<T>::BeginItem(key, w);
  if (!w->IsSelected()) {
    return {w, false};
  }
  const auto nav
    = static_cast<Widgets::NavigationView*>(w->GetSelectionContainer());
  if (selectedThisFrame) {
    nav->SetHeaderText(headerText);
  }

  PushParentOverride(nav->GetContentRoot());

  return {w, true};
}

/** A NavigationViewItem with the fancy spinning gear on click/release
 *
 * The *only* difference to a normal NavigationViewItem is the spinning
 * animation.
 */
template <selectable_key T>
NavigationViewItemResult BeginNavigationViewSettingsItem(
  const T key,
  const std::string_view label = "Settings",
  const std::string_view headerText = "Settings",
  const ID id = ID {std::source_location::current()}) {
  using namespace immediate_detail;
  const auto w
    = immediate_detail::ChildlessWidget<Widgets::NavigationViewSettingsItem>(id)
        ->SetText(label);

  const auto selectedThisFrame = SelectionManager<T>::BeginItem(key, w);
  if (!w->IsSelected()) {
    return {w, false};
  }
  const auto nav
    = static_cast<Widgets::NavigationView*>(w->GetSelectionContainer());
  if (selectedThisFrame) {
    nav->SetHeaderText(headerText);
  }

  PushParentOverride(nav->GetContentRoot());

  return {w, true};
}

inline void EndNavigationViewSettingsItem() {
  EndNavigationViewItem();
}

template <selectable_key T>
NavigationViewItemResult BeginNavigationViewItem(
  const T key,
  const std::string_view icon,
  const std::string_view label,
  const ID id = ID {std::source_location::current()}) {
  return BeginNavigationViewItem(key, icon, label, label, id);
}

inline void EndNavigationViewFooterItems() {
  immediate_detail::PopParentOverride();
}

inline Result<
  &EndNavigationViewFooterItems,
  void,
  immediate_detail::WidgetlessResultMixin>
BeginNavigationViewFooterItems() {
  using namespace immediate_detail;
  const auto nav = GetCurrentParentNode()// non-footer items
                     ->GetStructuralParent()// pane
                     ->GetStructuralParent<Widgets::NavigationView>();
  PushParentOverride(nav->GetFooterItemsRoot());
  return {};
}

}// namespace FredEmmott::GUI::Immediate