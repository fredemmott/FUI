// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate/TextBlockStylesMixin.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

using TextBlockResult
  = Result<nullptr, void, immediate_detail::TextBlockStylesMixin>;

TextBlockResult TextBlock(
  std::string_view text,
  ID id = ID {std::source_location::current()});

template <class... Args>
TextBlockResult TextBlock(std::format_string<Args...> fmt, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(fmt, std::forward<Args>(args)...);
  return TextBlock(text, id);
}

}// namespace FredEmmott::GUI::Immediate