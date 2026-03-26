// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndMenuFlyout();
using MenuFlyoutResult = Result<
  &EndMenuFlyout,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopeableResultMixin>;

[[nodiscard]]
MenuFlyoutResult BeginMenuFlyout(ID = ID {std::source_location::current()});
[[nodiscard]]
MenuFlyoutResult BeginMenuFlyout(
  bool* open,
  ID = ID {std::source_location::current()});

using MenuFlyoutItemResult = Result<nullptr, bool>;

[[nodiscard]]
MenuFlyoutItemResult MenuFlyoutItem(
  std::string_view glyph,
  std::string_view label,
  ID = ID {std::source_location::current()});

[[nodiscard]]
inline MenuFlyoutItemResult MenuFlyoutItem(
  const std::string_view label,
  const ID id = ID {std::source_location::current()}) {
  return MenuFlyoutItem({}, label, id);
}

template <class... Args>
[[nodiscard]]
MenuFlyoutItemResult MenuFlyoutItem(
  const std::string_view glyph,
  std::format_string<std::string_view> format,
  Args&&... args) {
  const auto [id, label]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return MenuFlyoutItem(glyph, label, id);
}

void MenuFlyoutSeparator(ID = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate