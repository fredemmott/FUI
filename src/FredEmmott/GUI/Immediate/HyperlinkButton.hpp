// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

Result<nullptr, void, immediate_detail::CaptionResultMixin> HyperlinkButton(
  bool* clicked,
  std::string_view label,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
Result<nullptr, bool, immediate_detail::CaptionResultMixin> HyperlinkButton(
  std::string_view label,
  ID id = ID {std::source_location::current()});

template <class... Args>
  requires(sizeof...(Args) >= 1)
[[nodiscard]]
Result<nullptr, bool, immediate_detail::CaptionResultMixin> HyperlinkButton(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text] = ParsedID(format, std::forward<Args>(args)...);
  return Button(text, id);
}

}// namespace FredEmmott::GUI::Immediate