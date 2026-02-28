// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string_view>

namespace FredEmmott::GUI::Immediate {

void WindowTitle(std::string_view);
/// Returns false if feature is unsupported or unavailable
[[nodiscard]]
bool WindowSubtitle(std::string_view);

}// namespace FredEmmott::GUI::Immediate