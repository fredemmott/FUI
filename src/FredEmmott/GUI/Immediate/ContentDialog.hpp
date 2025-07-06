// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "FredEmmott/GUI/detail/immediate/WidgetlessResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndContentDialog();
using ContentDialogResult = Result<
  &EndContentDialog,
  bool,
  immediate_detail::WidgetlessResultMixin,
  immediate_detail::ConditionallyScopedResultMixin>;

ContentDialogResult BeginContentDialog(
  std::string_view title,
  ID id = ID {std::source_location::current()});

ContentDialogResult BeginContentDialog(
  bool* open,
  std::string_view title,
  ID id = ID {std::source_location::current()});

template <class... Args>
  requires(sizeof...(Args) > 0)
ContentDialogResult BeginContentDialog(
  std::format_string<Args...> titleFormat,
  Args&&... args) {
  auto [id, title]
    = immediate_detail::ParsedID(titleFormat, std::forward<Args>(args)...);
  return BeginContentDialog(title, id);
}

template <class... Args>
  requires(sizeof...(Args) > 0)
ContentDialogResult BeginContentDialog(
  bool* open,
  std::format_string<Args...> titleFormat,
  Args&&... args) {
  if (!(open && *open)) {
    return false;
  }
  auto [id, title]
    = immediate_detail::ParsedID(titleFormat, std::forward<Args>(args)...);
  return BeginContentDialog(open, title, id);
}

void EndContentDialogFooter();
Result<&EndContentDialogFooter, void, immediate_detail::WidgetlessResultMixin>
BeginContentDialogFooter();

}// namespace FredEmmott::GUI::Immediate