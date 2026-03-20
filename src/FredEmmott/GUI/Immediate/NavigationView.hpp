// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/NavigationView.hpp>
#include <FredEmmott/GUI/Widgets/NavigationViewItem.hpp>
#include <FredEmmott/GUI/detail/immediate/SelectionManager.hpp>
#include <FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp>

#include "FredEmmott/GUI/Widgets/NavigationViewSettingsItem.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "Result.hpp"
#include "selectable_key.hpp"

namespace FredEmmott::GUI::Immediate {

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
  SelectionManager<T>::BeginContainer(w, selectedKey);
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