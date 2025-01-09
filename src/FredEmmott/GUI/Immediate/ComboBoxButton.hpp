// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

/** Start a button containing a child widget; for multiple widgets, use
 * a layout.
 *
 * Usage:
 *
 *   BeginComboBoxButton(&clicked, [style[, id]]);
 *
 * @see `ComboBoxButton()` if you just want text
 */
void BeginComboBoxButton(bool* clicked, ID id = ID {std::source_location::current()});

void EndComboBoxButton();

/// Create a button with a text label
[[nodiscard]]
bool ComboBoxButton(
  std::string_view label,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
template <class... Args>
[[nodiscard]] bool ComboBoxButton(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return ComboBoxButton(text, id);
}

}// namespace FredEmmott::GUI::Immediate
