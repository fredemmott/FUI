// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/NavigationView.hpp>
#include <FredEmmott/GUI/Widgets/NavigationViewItem.hpp>
#include <FredEmmott/GUI/detail/immediate/SelectionManager.hpp>
#include <FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp>

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
  const auto view
    = static_cast<Widgets::NavigationView*>(w->GetSelectionContainer());
  if (selectedThisFrame) {
    view->SetHeaderText(headerText);
  }

  PushParentOverride(view->GetContentRoot());

  return {w, true};
}

template <selectable_key T>
NavigationViewItemResult BeginNavigationViewItem(
  const T key,
  const std::string_view icon,
  const std::string_view label,
  const ID id = ID {std::source_location::current()}) {
  return BeginNavigationViewItem(key, icon, label, label, id);
}

}// namespace FredEmmott::GUI::Immediate