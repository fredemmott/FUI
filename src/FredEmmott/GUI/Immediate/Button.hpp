// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Button.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

/** Start a button containing a child widget; for multiple widgets, use
 * a layout.
 *
 * Usage:
 *
 *   BeginButton(&clicked, [style[, id]]);
 *
 * @see `Button()` if you just want text
 */
void BeginButton(bool* clicked, ID id = ID {std::source_location::current()});

inline void EndButton() {
  immediate_detail::EndWidget<Widgets::Button>();
}

/// Create a button with a text label
[[nodiscard]]
bool Button(
  std::string_view label,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
template <class... Args>
[[nodiscard]] bool Button(std::format_string<Args...> format, Args&&... args) {
  const auto [id, text] = ParsedID(format, std::forward<Args>(args)...);
  return Button(text, id);
}

}// namespace FredEmmott::GUI::Immediate
