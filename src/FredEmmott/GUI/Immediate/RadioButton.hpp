// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <type_traits>

#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "ID.hpp"
#include "Result.hpp"
#include "selectable_key.hpp"

namespace FredEmmott::GUI::Immediate {

void EndRadioButton();
[[nodiscard]]
Result<&EndRadioButton, bool> BeginRadioButton(
  bool isInitiallyChecked,
  ID id = ID {std::source_location::current()});

template <selectable_key T>
Result<&EndRadioButton, bool> BeginRadioButton(
  T* checkedIndex,
  const T index,
  const ID id = ID {std::source_location::current()}) {
  FUI_ASSERT(checkedIndex != nullptr);
  const auto activated = BeginRadioButton((index == *checkedIndex), id);
  if (activated) {
    *checkedIndex = index;
  }
  return activated;
}

[[nodiscard]]
Result<nullptr, bool> RadioButton(
  bool isInitiallyChecked,
  std::string_view label,
  ID = ID {std::source_location::current()});

template <selectable_key T>
Result<nullptr, bool> RadioButton(
  T* checkedIndex,
  const T index,
  const std::string_view label,
  const ID id = ID {std::source_location::current()}) {
  FUI_ASSERT(checkedIndex != nullptr);
  const auto activated = RadioButton((index == *checkedIndex), label, id);
  if (activated) {
    *checkedIndex = index;
  }
  return activated;
}

template <class... Args>
  requires(sizeof...(Args) > 0)
Result<nullptr, bool> RadioButton(
  const bool isInitiallyChecked,
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return RadioButton(isInitiallyChecked, text, id);
}

template <selectable_key T, class... Args>
  requires(sizeof...(Args) > 0)
  && ((!std::same_as<ID, std::decay_t<Args>>) && ...)
Result<nullptr, bool> RadioButton(
  T* checkedIndex,
  const T index,
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return RadioButton(checkedIndex, index, text, id);
}

}// namespace FredEmmott::GUI::Immediate