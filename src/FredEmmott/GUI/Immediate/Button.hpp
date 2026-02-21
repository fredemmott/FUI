// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Result.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>
#include <FredEmmott/GUI/Widgets/Button.hpp>
#include <FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

struct ButtonResultMixin : CaptionResultMixin {
  template <class Self>
  decltype(auto) Accent(this Self&& self) {
    widget_from_result(self)->AddStyleClass(
      StaticTheme::Button::AccentButtonStyleClass);
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) DefaultAction(this Self&& self) {
    tWindow->SetDefaultAction([w = widget_from_result(self)] {
      if (!w->IsDisabled()) {
        w->Invoke();
      }
    });
    return std::forward<Self>(self);
  }

  template <class Self>
  decltype(auto) CancelAction(this Self&& self) {
    tWindow->SetCancelAction([w = widget_from_result(self)] {
      if (!w->IsDisabled()) {
        w->Invoke();
      }
    });
    return std::forward<Self>(self);
  }
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail

namespace FredEmmott::GUI::Immediate {

inline void EndButton() {
  immediate_detail::EndWidget<Widgets::Button>();
}

template <void (*TEndWidget)() = nullptr, class TValue = void, class... TMixins>
using ButtonResult = Result<
  TEndWidget,
  TValue,
  immediate_detail::ButtonResultMixin,
  immediate_detail::ToolTipResultMixin,
  TMixins...>;

/** Start a button containing a child widget; for multiple widgets, use
 * a layout.
 *
 * Usage:
 *
 *   BeginButton(&clicked, [style[, id]]);
 *
 * @see `Button(...)` if you just want text
 */
ButtonResult<&EndButton> BeginButton(
  bool* clicked,
  ID id = ID {std::source_location::current()});

/** Start a button containing a child widget.
 *
 * Returns truthy result if the button was clicked.
 *
 * The result can not be `Scoped()`, to prevent the following bug:
 *
 * ```
 * if (BeginButton.Scoped()) {
 *   Label("ButtonText");
 * }
 * ```
 *
 * This would be a bug because the if block is a click handler, i.e. the button
 * would not have a label except for the frame in which it is clicked.
 */
ButtonResult<&EndButton, bool, immediate_detail::UnscopeableResultMixin>
BeginButton(ID id = ID {std::source_location::current()});

/// Create a button with a text label
[[nodiscard]]
ButtonResult<nullptr, bool> Button(
  std::string_view label,
  ID id = ID {std::source_location::current()});

/// Create a button with a text label
template <class... Args>
  requires(sizeof...(Args) >= 1)
[[nodiscard]] ButtonResult<nullptr, bool> Button(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text] = ParsedID(format, std::forward<Args>(args)...);
  return Button(text, id);
}

}// namespace FredEmmott::GUI::Immediate
