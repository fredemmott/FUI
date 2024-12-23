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

}// namespace FredEmmott::GUI::Widgets