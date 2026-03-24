// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Root.hpp"

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <felly/scope_exit.hpp>

#include "FredEmmott/GUI/StaticTheme.hpp"
#include "FredEmmott/GUI/detail/immediate/Widget.hpp"
#include "FredEmmott/utility/almost_equal.hpp"

using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;
using namespace Widgets;

Root::Root(Widgets::Widget* root, Widgets::Widget* immediateRoot)
  : mActualRoot(root),
    mImmediateRoot(immediateRoot),
    mFocusManager(immediateRoot),
    mYogaRoot(YGNodeNew()) {
  const auto yoga = mYogaRoot.get();
  YGNodeInsertChild(yoga, root->GetLayoutNode(), 0);
  YGNodeStyleSetFlexDirection(yoga, YGFlexDirectionRow);
  YGNodeStyleSetOverflow(yoga, YGOverflowVisible);
}

Root::~Root() {}

void Root::Reset() {
  mImmediateRoot->SetStructuralChildren({});
}

void Root::BeginFrame() {
  if (!tStack.empty()) [[unlikely]] {
    throw std::logic_error(
      "BeginFrame() called, but frame already in progress");
  }

  PushParentOverride(mImmediateRoot);
}

void Root::EndFrame() {
  if (tStack.size() != 2) {
    throw std::logic_error("EndFrame() called, but children are open");
  }
  PopParentOverride();
  FUI_ASSERT(tStack.empty());

  FUI_ASSERT(
    mImmediateRoot->GetStructuralChildren().size() == 1,
    "Immediate widget root must have a single child, usually a layout or "
    "card");
  mActualRoot->ComputeStyles({});

  if (tResizeThisFrame) {
    tWindow->ResizeToIdeal();
  }
  tResizeThisFrame = std::exchange(tResizeNextFrame, false);
}

Widget* Root::DispatchEvent(const Event& e) {
  return mActualRoot->DispatchEvent(e);
}

void Root::Paint(Renderer* renderer, const Size& size) {
  const auto clipRegion = renderer->ScopedClipRect({size});

  const auto frameStartTime = std::chrono::steady_clock::now();

  YGNodeCalculateLayout(
    this->GetLayoutNode(), size.mWidth, size.mHeight, YGDirectionLTR);

  if constexpr (Config::Debug) {
    const auto width = YGNodeLayoutGetWidth(this->GetLayoutNode());
    FUI_ALWAYS_ASSERT(!std::isnan(width));
    FUI_ALWAYS_ASSERT(width > 0);
    FUI_ALWAYS_ASSERT(std::abs(width - size.mWidth) < 1.0f);
    const auto height = YGNodeLayoutGetHeight(this->GetLayoutNode());
    FUI_ALWAYS_ASSERT(!std::isnan(height));
    FUI_ALWAYS_ASSERT(height > 0);
    FUI_ALWAYS_ASSERT(std::abs(height - size.mHeight) < 1.0f);
  }

  mActualRoot->Tick(frameStartTime);
  mActualRoot->Paint(renderer);
}

bool Root::CanFit(const Size& size) const {
  return CanFit(size.mWidth, size.mHeight);
}

bool Root::CanFit(float width, float height) const {
  if (width <= 0 || height <= 0) {
    return false;
  }
  const auto yoga = this->GetLayoutNode();
  const auto checkParents = ScopedYogaParentCheck(yoga);
  const auto fixParents = felly::scope_exit([yoga] { FixYogaChildren(yoga); });
  const unique_yoga_node_ptr testRoot {YGNodeClone(yoga)};
  const auto cloned = testRoot.get();
  YGNodeStyleSetWidth(cloned, std::ceil(width));
  YGNodeStyleSetHeight(cloned, std::ceil(height));
  YGNodeCalculateLayout(cloned, YGUndefined, YGUndefined, YGDirectionLTR);
  return !YGNodeLayoutGetHadOverflow(cloned);
}

YGNodeRef Root::GetLayoutNode() const {
  return mYogaRoot.get();
}

FocusManager* Root::GetFocusManager() const {
  return &const_cast<Root*>(this)->mFocusManager;
}

Size Root::GetInitialSize() const {
  return GetMinimumWidthAndIdealHeight(this->GetLayoutNode());
}

float Root::GetHeightForWidth(float width) const {
  return GetIdealHeight(this->GetLayoutNode(), width);
}

FrameRateRequirement Root::GetFrameRateRequirement() const {
  if (std::exchange(tNeedAdditionalFrame, false)) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return mActualRoot->GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Immediate