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
 * @see `Button()` if you just want text
 */

constexpr immediate_detail::BeginWidget<Widgets::Button> BeginButton;
inline void EndButton() {
  immediate_detail::EndWidget();
}

bool IsPreviousButtonClicked();

/// Create a button with options and a text label
template <class... Args>
[[nodiscard]] bool Button(
  const Widgets::WidgetStyles& styles,
  std::format_string<Args...> format,
  Args&&... args) {
  using namespace immediate_detail;

  const auto [id, text]
    = ParsedID::Make<Widgets::Button>(format, std::forward<Args>(args)...);
  BeginButton(styles, id);
  Label(
    WidgetStyles {
      .mDefault = {
        .mFont = WidgetFont::ControlContent,
      },
      } + styles.InheritableStyles(),
    "{}##Label",
    text);
  EndButton();
  return IsPreviousButtonClicked();
}

/// Create a button with a text label
template <class... Args>
[[nodiscard]] bool Button(std::format_string<Args...> format, Args&&... args) {
  return Button({}, format, std::forward<Args>(args)...);
}

}// namespace FredEmmott::GUI::Immediate
