// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/CheckBox.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {
inline void EndCheckBox() {
  immediate_detail::EndWidget<Widgets::CheckBox>();
}

Result<&EndCheckBox> BeginCheckBox(
  bool* isChanged,
  bool* isChecked,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
Result<nullptr, bool> CheckBox(
  bool* isChecked,
  std::string_view label,
  ID id = ID {std::source_location::current()});

template <class... Args>
[[nodiscard]]
Result<nullptr, bool>
CheckBox(bool* isChecked, std::format_string<Args...> format, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return CheckBox(isChecked, text, id);
}

}// namespace FredEmmott::GUI::Immediate