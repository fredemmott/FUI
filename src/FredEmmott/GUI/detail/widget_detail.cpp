// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "widget_detail.hpp"

#include <ranges>

namespace FredEmmott::GUI::Widgets::widget_detail {

void AssignChildren(
  std::vector<unique_ptr<Widget>>* store,
  const std::vector<Widget*>& childPointers) {
  std::vector<unique_ptr<Widget>> newChildren;
  for (auto child: childPointers) {
    auto it = std::ranges::find(*store, child, &unique_ptr<Widget>::get);
    if (it == store->end()) {
      newChildren.emplace_back(child);
    } else {
      newChildren.emplace_back(std::move(*it));
    }
  }
  *store = std::move(newChildren);
}

}// namespace FredEmmott::GUI::Widgets::widget_detail