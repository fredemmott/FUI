// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Layout.hpp"

#include <FredEmmott/GUI/detail/widget_detail.hpp>

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

void Layout::SetChildren(const std::vector<Widget*>& children) {
  if (children == mChildRawPointers) {
    return;
  }

  const auto layout = this->GetLayoutNode();
  YGNodeRemoveAllChildren(layout);

  AssignChildren(&mChildren, children);
  mChildRawPointers = std::move(children);

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