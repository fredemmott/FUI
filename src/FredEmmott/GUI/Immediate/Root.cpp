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

  mWidget->Tick();
  mWidget->ComputeStyles({});
  mWidget->UpdateLayout();
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
  const auto clipRegion
    = renderer->ScopedClipRect({.mSize = {size.mWidth, size.mHeight}});

  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, size.mWidth, YGUndefined, YGDirectionLTR);
  mWidget->Paint(renderer);
}
bool Root::CanFit(const Size& size) const {
  return CanFit(size.mWidth, size.mHeight);
}

bool Root::CanFit(float width, float height) const {
  return GetHeightForWidth(width) <= height;
}

Size Root::GetInitialSize() const {
  if (mWidget) {
    mWidget->UpdateLayout();
    mWidget->ComputeStyles({});
  }
  float minWidth = 128;
  float maxWidth = 2048;
  while ((maxWidth - minWidth) > 1) {
    // We clone the node due to caching bugs with YGNodeCalculateLayout with
    // varying sizes, and instead set the width property on the clone.
    //
    // This workaround is suggested here:
    //
    // https://github.com/facebook/yoga/issues/1003#issuecomment-642888983
    unique_ptr<YGNode> testRoot {YGNodeClone(mYogaRoot.get())};
    const auto yoga = testRoot.get();

    const auto mid = std::ceil((minWidth + maxWidth) / 2);
    YGNodeStyleSetWidth(yoga, mid);
    YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
    if (YGNodeLayoutGetHadOverflow(yoga)) {
      if (mid - minWidth < 1) {
        break;
      }
      minWidth = mid;
    } else {
      if (maxWidth - mid < 1) {
        break;
      }
      maxWidth = mid;
    }
  }

  unique_ptr<YGNode> testRoot {YGNodeClone(mYogaRoot.get())};
  const auto yoga = testRoot.get();
  YGNodeStyleSetWidth(yoga, maxWidth);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);

  return Size {
    std::ceil(YGNodeLayoutGetWidth(yoga)),
    std::ceil(YGNodeLayoutGetHeight(yoga)),
  };
}

float Root::GetHeightForWidth(float width) const {
  unique_ptr<YGNode> testRoot {YGNodeClone(mYogaRoot.get())};
  auto yoga = testRoot.get();
  YGNodeStyleSetWidth(yoga, width);
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  const auto h = YGNodeLayoutGetHeight(yoga);
  return h;
}

FrameRateRequirement Root::GetFrameRateRequirement() const {
  if (tNeedAdditionalFrame.TestAndClear() || !mWidget) {
    return FrameRateRequirement::SmoothAnimation;
  }
  return mWidget->GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Immediate