// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <format>

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

void TextBlock(std::string_view text, ID id);

template <class... Args>
void TextBlock(std::format_string<Args...> fmt, Args&&... args) {
  const auto [id, text]
    = immediate_detail::ParsedID(fmt, std::forward<Args>(args)...);
  return TextBlock(text, id);
}

}// namespace FredEmmott::GUI::Immediate