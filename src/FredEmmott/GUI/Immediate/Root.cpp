// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Root.hpp"

namespace FredEmmott::GUI::Immediate {

using namespace immediate_detail;
using namespace Widgets;

void Root::BeginFrame() {
  if (!tStack.empty()) {
    throw std::logic_error(
      "BeginFrame() called, but frame already in progress");
  }

  tStack.push_back({});
  if (mWidget) {
    tStack.front().mChildren.push_back(mWidget.get());
  }
}

void Root::EndFrame(SkScalar w, SkScalar h, SkCanvas* canvas) {
  if (tStack.size() != 1) {
    throw std::logic_error("EndFrame() called, but children are open");
  }
  const auto widgets = tStack.front().mChildren;
  tStack.clear();

  if (widgets.empty()) {
    mWidget = nullptr;
    this->Paint(w, h, canvas);
    return;
  }

  if (widgets.size() > 1) {
    throw std::logic_error(
      "Root must have a single child, usually a layout or card");
  }

  const auto widget = widgets.front();
  if (widget != mWidget.get()) {
    mWidget.reset(widget);
  }

  this->Paint(w, h, canvas);
}

void Root::Paint(SkScalar w, SkScalar h, SkCanvas* canvas) const {
  if (!mWidget) {
    return;
  }
  YGNodeCalculateLayout(mWidget->GetLayoutNode(), w, h, YGDirectionLTR);
  mWidget->Paint(canvas);
}

}// namespace FredEmmott::GUI::Immediate