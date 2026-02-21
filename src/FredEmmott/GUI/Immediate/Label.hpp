// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/TextBlockStylesResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

using LabelResult
  = Result<nullptr, void, immediate_detail::TextBlockStylesResultMixin>;

LabelResult Label(
  std::string_view text,
  ID id = ID {std::source_location::current()});

template <class... Args>
LabelResult Label(std::format_string<Args...> fmt, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(fmt, std::forward<Args>(args)...);
  return Label(text, id);
}

}// namespace FredEmmott::GUI::Immediate