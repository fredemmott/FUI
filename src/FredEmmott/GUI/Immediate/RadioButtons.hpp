// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {
void EndRadioButtons();
/** Container for multiple RadioButton items.
 *
 * Not strictly required, however:
 * - without it, keyboard navigation (tabs/arrow keys) won't work
 * - it sets the flexbox gap property correctly
 * - optionally, it adds a header with the correct margin.
 */
Result<&EndRadioButtons> BeginRadioButtons(
  std::string_view title = {},
  ID = ID {std::source_location::current()});
}// namespace FredEmmott::GUI::Immediate
