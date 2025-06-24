// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Root.hpp"

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>

#include "FredEmmott/GUI/StaticTheme.hpp"

using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Immediate {

using namespace immediate_detail;
using namespace Widgets;

Root::Root() {
  mYogaRoot.reset(YGNodeNew());
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

void Root::Paint(SkCanvas* canvas, const Size& size) {
  if (!mWidget) {
    return;
  }
  canvas->save();
  canvas->clipRect(Rect {.mSize = {size.mWidth, size.mHeight}});

  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, size.mWidth, size.mHeight, YGDirectionLTR);
  mWidget->Paint(canvas);

  canvas->restore();
}
bool Root::CanFit(const Size& size) const {
  return CanFit(size.mWidth, size.mHeight);
}

bool Root::CanFit(float width, float height) const {
  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, width, height, YGDirectionLTR);
  if (YGNodeLayoutGetHadOverflow(yoga)) {
    return false;
  }
  return true;
}

Size Root::GetInitialSize() const {
  if (mWidget) {
    mWidget->UpdateLayout();
    mWidget->ComputeStyles({});
  }
  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
  // This works as for things with a variable width (e.g. wrappable text), we
  // return `YGUndefined` from the measure functions if the width is undefined.
  //
  // While we 'should' return the true maximum size (e.g the length of all the
  // text as a single line), that unfortunately is interpreted as having a
  // definite size, as Yoga does not currently support measure funcs providing
  // minimum values.
  return Size {
    YGNodeLayoutGetWidth(yoga),
    YGNodeLayoutGetHeight(yoga),
  };
}

float Root::GetHeightForWidth(float width) const {
  auto yoga = mYogaRoot.get();
  YGNodeCalculateLayout(yoga, width, YGUndefined, YGDirectionLTR);
  return YGNodeLayoutGetHeight(yoga);
}

FrameRateRequirement Root::GetFrameRateRequirement() const {
  if (tNeedAdditionalFrame.TestAndClear() || !mWidget) {
    return FrameRateRequirement::SmoothAnimation;
  }
  return mWidget->GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Immediate