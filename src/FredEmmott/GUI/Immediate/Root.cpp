// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Root.hpp"

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <print>

#include "FredEmmott/GUI/StaticTheme.hpp"

using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;
using namespace Widgets;

Root::Root() {
  mYogaRoot.reset(YGNodeNew());
  const auto yoga = mYogaRoot.get();
  YGNodeStyleSetFlexDirection(yoga, YGFlexDirectionRow);
  YGNodeStyleSetOverflow(yoga, YGOverflowVisible);
}
Root::~Root() {}

void Root::BeginFrame() {
  if (!tStack.empty()) {
    throw std::logic_error(
      "BeginFrame() called, but frame already in progress");
  }

  tStack.push_back({});
  if (mWidget) {
    tStack.front().mPending.push_back(mWidget.get());
  }
}

void Root::EndFrame() {
  if (tStack.size() != 1) {
    throw std::logic_error("EndFrame() called, but children are open");
  }
  const auto widgets = std::move(tStack.front().mNewSiblings);
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

  if (tResizeThisFrame) {
    tWindow->ResizeToIdeal();
  }
  tResizeThisFrame = std::exchange(tResizeNextFrame, false);
}

void Root::DispatchEvent(const Event* e) {
  if (mWidget) {
    mWidget->DispatchEvent(e);
  }
}

void Root::Paint(Renderer* renderer, const Size& size) {
  if (!mWidget) {
    return;
  }
  const auto clipRegion = renderer->ScopedClipRect({size});

  mWidget->ComputeStyles({});
  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, size.mWidth, size.mHeight, YGDirectionLTR);

  mWidget->UpdateLayout();
  mWidget->Tick();
  mWidget->Paint(renderer);
}

bool Root::CanFit(const Size& size) const {
  return CanFit(size.mWidth, size.mHeight);
}

bool Root::CanFit(float width, float height) const {
  if (width <= 0 || height <= 0) {
    return false;
  }
  const unique_ptr<YGNode> testRoot {YGNodeClone(mYogaRoot.get())};
  const auto yoga = testRoot.get();
  YGNodeStyleSetWidth(yoga, std::ceil(width));
  YGNodeStyleSetHeight(yoga, std::ceil(height));
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  return !YGNodeLayoutGetHadOverflow(yoga);
}
YGNodeRef Root::GetLayoutNode() const {
  return mYogaRoot.get();
}

Size Root::GetInitialSize() const {
  if (mWidget) {
    mWidget->ComputeStyles({});
    mWidget->UpdateLayout();
  }

  return GetMinimumWidthAndIdealHeight(mYogaRoot.get());
}

float Root::GetHeightForWidth(float width) const {
  return GetIdealHeight(mYogaRoot.get(), width);
}

FrameRateRequirement Root::GetFrameRateRequirement() const {
  if (tNeedAdditionalFrame.TestAndClear() || !mWidget) {
    return FrameRateRequirement::SmoothAnimation;
  }
  return mWidget->GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Immediate