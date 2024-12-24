// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

void EndWidget() {
  const auto back = std::move(tStack.back());
  tStack.pop_back();

  GetCurrentNode()->SetChildren(back.mChildren);
  ++tStack.back().mNextIndex;
}

}// namespace FredEmmott::GUI::Immediate::immediate_detail
