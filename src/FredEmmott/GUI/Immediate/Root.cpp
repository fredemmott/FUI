// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Root.hpp"

#include <wil/resource.h>

#include <FredEmmott/GUI/Color.hpp>

#include "FredEmmott/GUI/StaticTheme.hpp"

using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Immediate {

using namespace immediate_detail;
using namespace Widgets;

Root::Root() {
  mYogaRoot.reset(YGNodeNew());
}

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

void Root::EndFrame() {
  if (tStack.size() != 1) {
    throw std::logic_error("EndFrame() called, but children are open");
  }
  const auto widgets = tStack.front().mChildren;
  tStack.clear();

  if (widgets.empty()) {
    mWidget = nullptr;
    return;
  }

  if (widgets.size() > 1) {
    throw std::logic_error(
      "Root must have a single child, usually a layout or card");
  }

  const auto widget = widgets.front();
  if (widget != mWidget.get()) {
    mWidget.reset(widget);
    auto node = mWidget->GetLayoutNode();
    YGNodeSetChildren(mYogaRoot.get(), &node, 1);
  }

  mWidget->ComputeStyles({});
}

void Root::DispatchEvent(const Event* e) {
  if (mWidget) {
    mWidget->DispatchEvent(e);
  }
}

void Root::Paint(SkCanvas* canvas, SkSize size) const {
  if (!mWidget) {
    return;
  }
  canvas->save();
  canvas->clipRect(SkRect::MakeXYWH(0, 0, size.width(), size.height()));

  YGNodeCalculateLayout(
    mYogaRoot.get(), size.width(), size.height(), YGDirectionLTR);
  mWidget->Paint(canvas);

  canvas->restore();
}

std::optional<SkSize> Root::GetMinimumSize() const {
  if (!mWidget) {
    return std::nullopt;
  }

  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  return SkSize {
    YGNodeLayoutGetWidth(yoga),
    YGNodeLayoutGetHeight(yoga),
  };
}

}// namespace FredEmmott::GUI::Immediate