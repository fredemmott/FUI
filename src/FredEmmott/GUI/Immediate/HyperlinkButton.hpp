// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

template <class TValue>
using HyperlinkButtonResult = Result<
  nullptr,
  TValue,
  immediate_detail::CaptionResultMixin,
  immediate_detail::TextBlockStylesResultMixin,
  immediate_detail::ToolTipResultMixin>;

HyperlinkButtonResult<void> HyperlinkButton(
  bool* clicked,
  std::string_view label,
  ID id = ID {std::source_location::current()});

[[nodiscard]]
HyperlinkButtonResult<bool> HyperlinkButton(
  std::string_view label,
  ID id = ID {std::source_location::current()});

template <class... Args>
  requires(sizeof...(Args) >= 1)
[[nodiscard]]
HyperlinkButtonResult<bool> HyperlinkButton(
  std::format_string<Args...> format,
  Args&&... args) {
  const auto [id, text] = ParsedID(format, std::forward<Args>(args)...);
  return Button(text, id);
}

}// namespace FredEmmott::GUI::Immediate