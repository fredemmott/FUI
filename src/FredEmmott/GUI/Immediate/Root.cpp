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
  if (mWidgetRoot) {
    tStack.front().mPending.push_back(mWidgetRoot->mWidget.get());
  }
}

void Root::EndFrame() {
  if (tStack.size() != 1) {
    throw std::logic_error("EndFrame() called, but children are open");
  }
  const auto widgets = std::move(tStack.front().mNewSiblings);
  tStack.clear();

  if (widgets.empty()) {
    mWidgetRoot.reset();
    return;
  }

  if (widgets.size() > 1) {
    throw std::logic_error(
      "Root must have a single child, usually a layout or card");
  }

  const auto widget = widgets.front();
  if ((!mWidgetRoot) || (mWidgetRoot->mWidget.get() != widget)) {
    mWidgetRoot.emplace(
      unique_ptr<Widgets::Widget>(widget), FocusManager {widget});
    const auto yoga = widget->GetLayoutNode();
    YGNodeSetChildren(mYogaRoot.get(), &yoga, 1);
  }

  if (tResizeThisFrame) {
    tWindow->ResizeToIdeal();
  }
  tResizeThisFrame = std::exchange(tResizeNextFrame, false);
}

Widget* Root::DispatchEvent(const Event* e) {
  if (mWidgetRoot) {
    return mWidgetRoot->mWidget->DispatchEvent(*e);
  }
  return nullptr;
}

void Root::Paint(Renderer* renderer, const Size& size) {
  if (!mWidgetRoot) {
    return;
  }
  const auto widget = mWidgetRoot->mWidget.get();
  const auto clipRegion = renderer->ScopedClipRect({size});

  FocusManager::PushInstance(&mWidgetRoot->mFocusManager);
  const auto popFocusManager = wil::scope_exit([this] {
    FocusManager::PopInstance(&mWidgetRoot->mFocusManager);
  });

  const auto frameStartTime = std::chrono::steady_clock::now();

  widget->ComputeStyles({});
  YGNodeCalculateLayout(
    mYogaRoot.get(), size.mWidth, size.mHeight, YGDirectionLTR);

  widget->Tick(frameStartTime);
  widget->Paint(renderer);
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

Widgets::Widget* Root::GetWidget() const {
  if (mWidgetRoot) {
    return mWidgetRoot->mWidget.get();
  }
  return nullptr;
}

FocusManager* Root::GetFocusManager() const {
  if (mWidgetRoot) {
    return const_cast<FocusManager*>(&mWidgetRoot->mFocusManager);
  }
  return nullptr;
}

Size Root::GetInitialSize() const {
  if (mWidgetRoot) {
    const auto widget = mWidgetRoot->mWidget.get();
    widget->ComputeStyles({});
  }

  return GetMinimumWidthAndIdealHeight(mYogaRoot.get());
}

float Root::GetHeightForWidth(float width) const {
  return GetIdealHeight(mYogaRoot.get(), width);
}

FrameRateRequirement Root::GetFrameRateRequirement() const {
  if (std::exchange(tNeedAdditionalFrame, false) || !mWidgetRoot) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return mWidgetRoot->mWidget->GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Immediate