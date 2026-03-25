// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "WasActivated.hpp"

#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {
bool WasActivated() noexcept {
  return immediate_detail::GetCurrentNode()->ConsumeWasActivated();
}

bool WasContextActivated() noexcept {
  return immediate_detail::GetCurrentNode()->ConsumeWasContextActivated();
}

}// namespace FredEmmott::GUI::Immediate