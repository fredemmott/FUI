// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Result.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>
#include <FredEmmott/GUI/Widgets/Button.hpp>
#include <FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Button.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct ButtonResultMixin : CaptionResultMixin {
  template <class Self>
  decltype(auto) Accent(this Self&& self) {
    widget_from_result(self)->SetAdditionalBuiltInStyles(
      StaticTheme::Button::AccentButtonStyle);
    return std::forward<Self>(self);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail

namespace FredEmmott::GUI::Immediate {

inline void EndButton() {
  immediate_detail::EndWidget<Widgets::Button>();
}

template <void (*TEndWidget)() = nullptr, class TValue = void>
using ButtonResult
  = Result<TEndWidget, TValue, immediate_detail::ButtonResultMixin>;

/** Start a button containing a child widget; for multiple widgets, use
 * a layout.
 *
 * Usage:
 *
 *   BeginButton(&clicked, [style[, id]]);
 *
 * @see `Button()` if you just want text
 */
ButtonResult<&EndButton> BeginButton(
  bool* clicked,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
[[nodiscard]]
ButtonResult<nullptr, bool> Button(
  std::string_view label,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
template <class... Args>
[[nodiscard]] ButtonResult<nullptr, bool> Button(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text] = ParsedID(format, std::forward<Args>(args)...);
  return Button(text, id);
}

}// namespace FredEmmott::GUI::Immediate
