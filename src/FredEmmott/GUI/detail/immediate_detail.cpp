// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

thread_local std::vector<StackEntry> tStack;

void TruncateUnlessNextIdEquals(const std::size_t id) {
  auto& [siblings, i, data] = tStack.back();
  if (i == siblings.size()) {
    return;
  }
  if (i > siblings.size()) [[unlikely]] {
    throw std::logic_error("Next sibling index > sibling count");
  }

  if (siblings.at(i)->GetID() == id) {
    return;
  }

  siblings.erase(siblings.begin() + i, siblings.end());
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail
