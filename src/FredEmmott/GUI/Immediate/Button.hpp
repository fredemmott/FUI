// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Result.hpp>
#include <FredEmmott/GUI/Widgets/Button.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "Button.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndButton() {
  immediate_detail::EndWidget<Widgets::Button>();
}

/** Start a button containing a child widget; for multiple widgets, use
 * a layout.
 *
 * Usage:
 *
 *   BeginButton(&clicked, [style[, id]]);
 *
 * @see `Button()` if you just want text
 */
Result<&EndButton> BeginButton(
  bool* clicked,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
[[nodiscard]]
Result<nullptr, bool> Button(
  std::string_view label,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
template <class... Args>
[[nodiscard]] Result<nullptr, bool> Button(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text] = ParsedID(format, std::forward<Args>(args)...);
  return Button(text, id);
}

}// namespace FredEmmott::GUI::Immediate
