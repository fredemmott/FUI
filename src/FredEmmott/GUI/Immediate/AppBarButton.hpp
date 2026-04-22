// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Immediate/Result.hpp>

#include "FredEmmott/GUI/detail/immediate/ToolTipResultMixin.hpp"
#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

using AppBarButtonResult
  = Result<nullptr, bool, immediate_detail::ToolTipResultMixin>;

[[nodiscard]]
AppBarButtonResult AppBarButton(
  std::string_view glyph,
  std::string_view label,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate