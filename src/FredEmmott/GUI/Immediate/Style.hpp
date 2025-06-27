// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

namespace FredEmmott::GUI::Immediate {
// style the current (previous) node.
inline void Style(const GUI::Style& style) {
  if (const auto previous = immediate_detail::GetCurrentNode()) {
    previous->ReplaceExplicitStyles(style);
  } else {
    immediate_detail::GetCurrentParentNode()->ReplaceExplicitStyles(style);
  }
}

}// namespace FredEmmott::GUI::Immediate
