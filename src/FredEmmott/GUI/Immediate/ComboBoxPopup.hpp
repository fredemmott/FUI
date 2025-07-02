// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {
void EndComboBoxPopup();

[[nodiscard]]
Result<&EndComboBoxPopup, bool, {.mHasWidgetPointer = false}>
BeginComboBoxPopup(ID id = ID {std::source_location::current()});
[[nodiscard]]
Result<&EndComboBoxPopup, bool, {.mHasWidgetPointer = false}>
BeginComboBoxPopup(bool* open, ID id = ID {std::source_location::current()});
}// namespace FredEmmott::GUI::Immediate