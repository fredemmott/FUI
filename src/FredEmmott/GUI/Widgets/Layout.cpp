// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Layout.hpp"

#include <unordered_set>

namespace FredEmmott::GUI::Widgets {

void Layout::SetChildren(const std::vector<Widget*>& children) {
  if (std::ranges::equal(children, this->GetChildren())) {
    return;
  }

  const auto layout = this->GetLayoutNode();
  YGNodeRemoveAllChildren(layout);

  std::vector<unique_ptr<Widget>> newChildren;
  for (auto child: children) {
    auto it = std::ranges::find(mChildren, child, &unique_ptr<Widget>::get);
    if (it == mChildren.end()) {
      newChildren.emplace_back(child);
    } else {
      newChildren.emplace_back(std::move(*it));
    }
  }
  mChildren = std::move(newChildren);

  const auto childLayouts
    = std::views::transform(mChildren, &Widget::GetLayoutNode)
    | std::ranges::to<std::vector>();
  YGNodeSetChildren(layout, childLayouts.data(), childLayouts.size());
}

void Layout::Paint(SkCanvas* canvas) const {
  const auto layout = this->GetLayoutNode();
  const auto x = YGNodeLayoutGetLeft(layout);
  const auto y = YGNodeLayoutGetTop(layout);

  canvas->save();
  canvas->translate(x, y);
  for (auto&& child: mChildren) {
    child->Paint(canvas);
  }
  canvas->restore();
}

}// namespace FredEmmott::GUI::Widgets