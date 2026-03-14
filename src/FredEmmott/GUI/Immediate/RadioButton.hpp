// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <type_traits>

#include "FredEmmott/GUI/Widgets/RadioButton.hpp"
#include "FredEmmott/GUI/detail/immediate/SelectionManager.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "ID.hpp"
#include "Result.hpp"
#include "selectable_key.hpp"

namespace FredEmmott::GUI::Immediate {

inline void EndRadioButton() {
  immediate_detail::EndWidget<Widgets::RadioButton>();
}

template <void (*TEndWidget)(), class TValue>
using RadioButtonResult
  = Result<TEndWidget, TValue, immediate_detail::ToolTipResultMixin>;

template <selectable_key T>
RadioButtonResult<&EndRadioButton, bool> BeginRadioButton(
  const T key,
  const ID id = ID {std::source_location::current()}) {
  using namespace immediate_detail;
  const auto widget = BeginWidget<Widgets::RadioButton>(id);
  const auto activated = SelectionManager<T>::BeginItem(key, widget);
  return {widget, activated};
}

template <selectable_key T>
RadioButtonResult<nullptr, bool> RadioButton(
  const T key,
  const std::string_view label,
  const ID id = ID {std::source_location::current()}) {
  const auto result = BeginRadioButton(key, id);
  Label(label);
  EndRadioButton();
  return result;
}

template <selectable_key T, class... Args>
  requires(sizeof...(Args) > 0)
RadioButtonResult<nullptr, bool>
RadioButton(const T key, std::format_string<Args...> format, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return RadioButton<T>(key, std::string_view {text}, id);
}

}// namespace FredEmmott::GUI::Immediate