// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "StackLayout.hpp"

#include <FredEmmott/GUI/detail/immediate_detail.hpp>

using namespace FredEmmott::GUI::Immediate::immediate_detail;

namespace FredEmmott::GUI::Immediate {

void BeginStackLayout(StackLayoutDirection direction) {
  const auto id = MakeID<StackLayout>(std::format(
    "{}/{}", tStack.back().mNextIndex, std::to_underlying(direction)));
  TruncateUnlessNextIdEquals(id);

  auto& [siblings, i] = tStack.back();
  if (i == siblings.size()) {
    siblings.push_back(new StackLayout(id, direction));
  }

  tStack.emplace_back(
    static_cast<StackLayout*>(siblings.at(i))->GetChildren()
    | std::ranges::to<std::vector>());
}

void EndStackLayout() {
  const auto back = std::move(tStack.back());
  tStack.pop_back();

  auto& [siblings, i] = tStack.back();
  static_cast<StackLayout*>(siblings.at(i))->SetChildren(back.mChildren);
  ++i;
}
}// namespace FredEmmott::GUI::Immediate