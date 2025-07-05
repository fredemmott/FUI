// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "Label.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

template <void (*TEndWidget)() = nullptr, class TValue = void>
using ComboBoxButtonResult
  = Result<TEndWidget, TValue, immediate_detail::CaptionResultMixin>;

void EndComboBoxButton();

/** Start a button containing a child widget; for multiple widgets, use
 * a layout.
 *
 * Usage:
 *
 *   BeginComboBoxButton(&clicked, [style[, id]]);
 *
 * @see `ComboBoxButton()` if you just want text
 */
ComboBoxButtonResult<&EndComboBoxButton> BeginComboBoxButton(
  bool* clicked,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
[[nodiscard]]
ComboBoxButtonResult<nullptr, bool> ComboBoxButton(
  std::string_view label,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
template <class... Args>
[[nodiscard]] auto ComboBoxButton(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return ComboBoxButton(text, id);
}

}// namespace FredEmmott::GUI::Immediate
