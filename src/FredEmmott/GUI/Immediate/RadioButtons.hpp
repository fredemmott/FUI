// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {
void EndRadioButtons();
/// Optional, not needed to use RadioButtons - however, it sets the correct
/// flexbox gap, and optionally adds a header with the correct margin.
Result<&EndRadioButtons> BeginRadioButtons(
  std::string_view title = {},
  ID = ID {std::source_location::current()});
}// namespace FredEmmott::GUI::Immediate
