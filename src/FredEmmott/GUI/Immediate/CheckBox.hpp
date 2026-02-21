// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/CheckBox.hpp>
#include <FredEmmott/GUI/detail/immediate/Widget.hpp>

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {
inline void EndCheckBox() {
  immediate_detail::EndWidget<Widgets::CheckBox>();
}

template <void (*TEndWidget)(), class TValue>
using CheckBoxResult = Result<
  TEndWidget,
  TValue,
  immediate_detail::CaptionResultMixin,
  immediate_detail::ToolTipResultMixin>;

CheckBoxResult<&EndCheckBox, void> BeginCheckBox(
  bool* isChanged,
  bool* isChecked,
  ID id = ID {std::source_location::current()});

CheckBoxResult<nullptr, bool> CheckBox(
  bool* isChecked,
  std::string_view label,
  ID id = ID {std::source_location::current()});

template <class... Args>
CheckBoxResult<nullptr, bool>
CheckBox(bool* isChecked, std::format_string<Args...> format, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(format, std::forward<Args>(args)...);
  return CheckBox(isChecked, text, id);
}

}// namespace FredEmmott::GUI::Immediate