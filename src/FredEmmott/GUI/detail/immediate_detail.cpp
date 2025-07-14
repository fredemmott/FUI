// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

thread_local std::vector<StackEntry> tStack;
thread_local Window* tWindow {nullptr};
thread_local bool tNeedAdditionalFrame {false};
thread_local bool tResizeThisFrame {false};
thread_local bool tResizeNextFrame {false};

}// namespace FredEmmott::GUI::Immediate::immediate_detail
