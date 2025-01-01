// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

thread_local std::vector<StackEntry> tStack;

}// namespace FredEmmott::GUI::Immediate::immediate_detail
