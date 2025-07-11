// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndComboBoxItem();
Result<&EndComboBoxItem> BeginComboBoxItem(
  bool* selected,
  bool initiallySelected,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
Result<nullptr, bool> ComboBoxItem(
  bool initiallySelected,
  std::string_view label,
  ID id = ID {std::source_location::current()});

template <class... Args>
[[nodiscard]]
Result<nullptr, bool> ComboBoxItem(
  bool initiallySelected,
  std::format_string<Args...> fmt,
  Args&&... args) {
  using namespace immediate_detail;
  const auto [id, label] = ParsedID {fmt, std::forward<Args>(args)...};
  return ComboBoxItem(initiallySelected, label, id);
}

}// namespace FredEmmott::GUI::Immediate