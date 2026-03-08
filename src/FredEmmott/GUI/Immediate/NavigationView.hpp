// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndNavigationView();

using NavigationViewResult = Result<&EndNavigationView, void>;

NavigationViewResult BeginNavigationView(
  const ID = ID {std::source_location::current()});

void EndNavigationViewItem();

using NavigationViewItemResult = Result<
  &EndNavigationViewItem,
  bool,
  immediate_detail::ToolTipResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin>;

NavigationViewItemResult BeginNavigationViewItem(
  std::string_view icon,
  std::string_view label,
  std::string_view headerText,
  ID id = ID {std::source_location::current()});

inline NavigationViewItemResult BeginNavigationViewItem(
  const std::string_view icon,
  const std::string_view label,
  const ID id = ID {std::source_location::current()}) {
  return BeginNavigationViewItem(icon, label, label, id);
}

}// namespace FredEmmott::GUI::Immediate