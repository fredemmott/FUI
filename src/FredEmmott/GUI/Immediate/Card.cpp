// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Card.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

void BeginCard(const CardOptions& options) {
  BeginCard(options, MakeID<Widgets::Card>());
}

void BeginCard(const CardOptions& options, MangledID id) {
  TruncateUnlessNextIdEquals(id);

  auto& [siblings, i] = tStack.back();
  if (i == siblings.size()) {
    siblings.push_back(new Widgets::Card(id, options));
  }

  const auto child = static_cast<Widgets::Card*>(siblings.at(i))->GetChild();
  if (child) {
    tStack.emplace_back(std::vector {child});
  } else {
    tStack.emplace_back();
  }
}

void EndCard() {
  const auto back = std::move(tStack.back());
  if (back.mChildren.size() > 1) [[unlikely]] {
    throw std::logic_error("Cards can only have a single child");
  }
  tStack.pop_back();

  auto& [siblings, i] = tStack.back();
  const auto button = static_cast<Widgets::Card*>(siblings.at(i));
  if (back.mChildren.empty()) {
    button->SetChild(nullptr);
  } else {
    button->SetChild(back.mChildren.back());
  }
  ++i;
}

}// namespace FredEmmott::GUI::Immediate