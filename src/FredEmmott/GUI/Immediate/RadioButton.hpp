// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>

#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

void EndRadioButton();
[[nodiscard]]
Result<&EndRadioButton, bool> BeginRadioButton(
  bool isInitiallyChecked,
  ID id = ID {std::source_location::current()});

template <std::unsigned_integral T>
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

template <std::unsigned_integral T>
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
Result<nullptr, bool> RadioButton(
  const bool isInitiallyChecked,
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return RadioButton(isInitiallyChecked, text, id);
}

template <std::unsigned_integral T, class... Args>
  requires((!std::same_as<ID, std::decay_t<Args>>) && ...)
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