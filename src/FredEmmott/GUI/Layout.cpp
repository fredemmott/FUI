// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Layout.hpp"

namespace FredEmmott::GUI {

void Layout::SetChildren(std::vector<Widget*>&& children) {
  const auto layout = this->GetLayoutNode();
  YGNodeRemoveAllChildren(layout);

  mChildren = std::move(children);

  const auto childLayouts
    = std::views::transform(mChildren, &Widget::GetLayoutNode)
    | std::ranges::to<std::vector>();
  YGNodeSetChildren(layout, childLayouts.data(), childLayouts.size());
}

void Layout::AppendChild(Widget* child) {
  mChildren.push_back(child);
  YGNodeInsertChild(
    this->GetLayoutNode(), child->GetLayoutNode(), mChildren.size() - 1);
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

}// namespace FredEmmott::GUI