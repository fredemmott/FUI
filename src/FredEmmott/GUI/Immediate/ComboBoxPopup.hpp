// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

[[nodiscard]]
bool BeginComboBoxPopup(ID id = ID { std::source_location::current() });
[[nodiscard]]
bool BeginComboBoxPopup(bool *open, ID id = ID { std::source_location::current() });
void EndComboBoxPopup();
}