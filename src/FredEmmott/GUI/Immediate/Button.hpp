// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Button.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Immediate {

using ButtonOptions = Widgets::Button::Options;

void BeginButton(bool* clicked, const ButtonOptions& options = {});
void BeginButton(
  bool* clicked,
  const ButtonOptions& options,
  immediate_detail::MangledID id);
void BeginButton(bool* clicked, const ButtonOptions& options, auto id) {
  using namespace immediate_detail;

  return BeginButton(clicked, options, MangledID {MakeID<Widgets::Button>(id)});
}

void EndButton();

template <class... Args>
[[nodiscard]] bool Button(
  const ButtonOptions& options,
  std::format_string<Args...> format,
  Args&&... args) {
  using namespace immediate_detail;

  const auto [id, text]
    = ParsedID::Make<Widgets::Button>(format, std::forward<Args>(args)...);
  bool clicked {};
  BeginButton(&clicked, options, id);
  Label(LabelOptions {.mFont = WidgetFont::ControlContent}, "{}##Label", text);
  EndButton();
  return clicked;
}

template <class... Args>
[[nodiscard]] bool Button(std::format_string<Args...> format, Args&&... args) {
  return Button(ButtonOptions {}, format, std::forward<Args>(args)...);
}

}// namespace FredEmmott::GUI::Immediate
