// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <type_traits>

#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

/** An identifier for the selected item in a set of radio buttons.
 *
 * This is primarily intended to allow ints, `std::size_t`, and
 * enums, but given all we need to allow is equality, it's fairly
 * permissive.
 *
 * - Pointers are disallowed to avoid confusion for parameters that
 *   take a pointer
 * - floats are disallowed as while they support ==, using == is
 *   usually a bad idea
 */
template <class T>
concept radio_button_key = std::equality_comparable<T>
  && std::is_trivially_copyable_v<T> && (!std::is_pointer_v<T>)
  && (!std::is_void_v<T>) && (!std::is_floating_point_v<T>);

void EndRadioButton();
[[nodiscard]]
Result<&EndRadioButton, bool> BeginRadioButton(
  bool isInitiallyChecked,
  ID id = ID {std::source_location::current()});

template <radio_button_key T>
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

template <radio_button_key T>
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

template <radio_button_key T, class... Args>
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