// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>
#include <print>

#include "FredEmmott/GUI/SystemSettings.hpp"
#include "FredEmmott/GUI/detail/widget_detail.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace widget_detail;

namespace {
const auto ScrollViewStyleClass = StyleClass::Make("ScrollView");
const auto ContentOuterStyleClass = StyleClass::Make("ScrollView_ContentOuter");

struct ScrollViewContext final : Context {
  explicit ScrollViewContext(ScrollView* sv) : mScrollView(sv) {}
  ScrollView* mScrollView {nullptr};
};

}// namespace

ScrollView::ScrollView(std::size_t id, const StyleClasses& classes)
  : Widget(id, classes + ScrollViewStyleClass) {
  this->ChangeDirectChildren([this] {
    mHorizontalScrollBar.reset(new ScrollBar(0, Orientation::Horizontal));
    mVerticalScrollBar.reset(new ScrollBar(1, Orientation::Vertical));
    mContentOuter.reset(new Widget(2, {ContentOuterStyleClass}));
    mContentInner.reset(new Widget(3));
  });
  YGNodeRemoveChild(this->GetLayoutNode(), mContentInner->GetLayoutNode());
  mContentYoga.reset(YGNodeNew());
  YGNodeInsertChild(mContentYoga.get(), mContentInner->GetLayoutNode(), 0);
  YGNodeSetContext(
    mContentYoga.get(),
    new YogaContext {DetachedYogaTree {
      .mLogicalParent = this,
      .mFosterParent = mContentInner.get(),
    }});

  YGNodeSetDirtiedFunc(
    mContentInner->GetLayoutNode(), &ScrollView::OnInnerContentDirty);
  mContentInner->SetContextIfUnset<ScrollViewContext>(this);
  YGNodeSetDirtiedFunc(
    mContentOuter->GetLayoutNode(), &ScrollView::OnOuterContentDirty);
  YGNodeSetMeasureFunc(
    mContentOuter->GetLayoutNode(), &ScrollView::MeasureOuterContent);
  mContentOuter->SetContextIfUnset<ScrollViewContext>(this);

  using StaticTheme::ScrollBar::ScrollBarSize;

  constexpr auto SmoothScrollingAnimation = CubicBezierStyleTransition(
    std::chrono::milliseconds(100),
    StaticTheme::Common::ControlFastOutSlowInKeySpline);
  mContentInner->SetBuiltInStyles({
    .mTranslateX = {0, SmoothScrollingAnimation},
    .mTranslateY = {0, SmoothScrollingAnimation},
  });
  mVerticalScrollBar->SetAdditionalBuiltInStyles({
    .mPosition = YGPositionTypeAbsolute,
    .mRight = 4,
    .mTop = 4,
  });
  mHorizontalScrollBar->SetAdditionalBuiltInStyles({
    .mBottom = 4,
    .mFlexGrow = 1,
    .mLeft = 0.f,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = ScrollBarSize,
  });

  mHorizontalScrollBar->OnValueChanged(
    std::bind_front(&ScrollView::OnHorizontalScroll, this));
  mVerticalScrollBar->OnValueChanged(
    std::bind_front(&ScrollView::OnVerticalScroll, this));
}

ScrollView::~ScrollView() {
  const auto ctx
    = static_cast<YogaContext*>(YGNodeGetContext(mContentYoga.get()));
  YGNodeSetContext(mContentYoga.get(), nullptr);
  delete ctx;
}

ScrollView::ScrollBarVisibility ScrollView::GetHorizontalScrollBarVisibility()
  const noexcept {
  return mHorizontalScrollBarVisibility;
}

ScrollView::ScrollBarVisibility ScrollView::GetVerticalScrollBarVisibility()
  const noexcept {
  return mVerticalScrollBarVisibility;
}

ScrollView& ScrollView::SetHorizontalScrollBarVisibility(
  const ScrollBarVisibility value) noexcept {
  mHorizontalScrollBarVisibility = value;
  return *this;
}

ScrollView& ScrollView::SetVerticalScrollBarVisibility(
  const ScrollBarVisibility value) noexcept {
  mVerticalScrollBarVisibility = value;
  return *this;
}

WidgetList ScrollView::GetDirectChildren() const noexcept {
  return {
    mVerticalScrollBar.get(),
    mHorizontalScrollBar.get(),
    mContentOuter.get(),
    mContentInner.get(),
  };
}

Widget* ScrollView::GetFosterParent() const noexcept {
  return mContentInner.get();
}

void ScrollView::UpdateLayout() {
  Widget::UpdateLayout();
  this->UpdateContentLayout();
}

void ScrollView::UpdateScrollBars() {
  const auto node = this->GetLayoutNode();
  mContentOuter->ComputeStyles({});

  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  if (YGFloatIsUndefined(w) || YGFloatIsUndefined(h)) {
    return;
  }

  const auto cw = YGNodeLayoutGetWidth(mContentInner->GetLayoutNode());
  const auto ch = YGNodeLayoutGetHeight(mContentInner->GetLayoutNode());

  const bool showHScroll
    = IsScrollBarVisible(mHorizontalScrollBarVisibility, cw, w);
  const bool showVScroll
    = IsScrollBarVisible(mVerticalScrollBarVisibility, ch, h);

  if (showHScroll) {
    mHorizontalScrollBar->SetThumbSize(w);
    mHorizontalScrollBar->SetMaximum(cw - w);
    mHorizontalScrollBar->AddExplicitStyles({.mDisplay = YGDisplayFlex});
    mVerticalScrollBar->AddExplicitStyles({
      .mBottom = StaticTheme::ScrollBar::ScrollBarSize + 4,
    });
  } else {
    mHorizontalScrollBar->SetValue(0);
    mHorizontalScrollBar->SetMaximum(0);
    mHorizontalScrollBar->AddExplicitStyles({.mDisplay = YGDisplayNone});
    mVerticalScrollBar->AddExplicitStyles({
      .mBottom = 4,
    });
  }

  if (showVScroll) {
    mVerticalScrollBar->SetThumbSize(h);
    mVerticalScrollBar->SetMaximum(ch - h);
    mVerticalScrollBar->AddExplicitStyles({.mDisplay = YGDisplayFlex});
  } else {
    mVerticalScrollBar->SetValue(0);
    mVerticalScrollBar->SetMaximum(0);
    mVerticalScrollBar->AddExplicitStyles({.mDisplay = YGDisplayNone});
  }
}

void ScrollView::PaintChildren(Renderer* renderer) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  {
    const auto clipTo = renderer->ScopedClipRect(Size {w, h});
    mContentInner->Paint(renderer);
  }

  mHorizontalScrollBar->Paint(renderer);
  mVerticalScrollBar->Paint(renderer);
}

void ScrollView::Tick() {
  Widget::Tick();
  this->UpdateContentLayout();
}

Widget::EventHandlerResult ScrollView::OnMouseVerticalWheel(
  const MouseEvent& e) {
  if (Widget::OnMouseVerticalWheel(e) == EventHandlerResult::StopPropagation) {
    return EventHandlerResult::StopPropagation;
  }

  const auto delta = std::get<MouseEvent::VerticalWheelEvent>(e.mDetail).mDelta;
  const auto lines = delta * SystemSettings::Get().GetMouseWheelScrollLines();
  const auto pixels
    = lines * SystemFont::Resolve(SystemFont::Body).GetMetrics().mSize;

  const auto scrollBar = mVerticalScrollBar.get();
  const auto value = std::clamp<float>(
    scrollBar->GetValue() + pixels, 0, scrollBar->GetMaximum());
  scrollBar->SetValue(value);
  return EventHandlerResult::StopPropagation;
}

Style ScrollView::GetBuiltInStyles() const {
  return Style {
    .mMinHeight = 128,
    .mMinWidth = 128,
  };
}

void ScrollView::OnHorizontalScroll(float value) {
  mContentInner->AddExplicitStyles({.mTranslateX = -value});
}

void ScrollView::OnVerticalScroll(float value) {
  mContentInner->AddExplicitStyles({.mTranslateY = -value});
}

bool ScrollView::IsScrollBarVisible(
  const ScrollBarVisibility visibility,
  const float content,
  const float container) noexcept {
  if (visibility == ScrollBarVisibility::Visible) {
    return true;
  }
  if (visibility == ScrollBarVisibility::Hidden) {
    return false;
  }

  constexpr auto eps = std::numeric_limits<float>::epsilon();
  if (content <= eps || container <= eps) {
    return false;
  }

  return (content - container) > eps;
}

void ScrollView::OnInnerContentDirty(YGNodeConstRef node) {
  auto& self
    = *FromYogaNode(node)->GetContext<ScrollViewContext>()->mScrollView;
  YGNodeMarkDirty(self.mContentOuter->GetLayoutNode());
}

void ScrollView::OnOuterContentDirty(YGNodeConstRef node) {
  auto& self = *static_cast<ScrollView*>(
    FromYogaNode(YGNodeGetParent(const_cast<YGNodeRef>(node))));
  self.mDirtyContentLayout = true;
}

YGSize ScrollView::MeasureOuterContent(
  YGNodeConstRef node,
  float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto outer = FromYogaNode(node);
  auto& self = *outer->GetContext<ScrollViewContext>()->mScrollView;

  const auto yoga = self.mContentYoga.get();
  const auto minWidth = GetMinimumWidth(yoga, YGNodeLayoutGetWidth(node));
  if (YGFloatIsUndefined(width)) {
    return {std::ceil(minWidth), std::ceil(GetIdealHeight(yoga, minWidth))};
  }

  if (width < minWidth) {
    width = minWidth;
  }

  if (heightMode == YGMeasureModeUndefined) {
    const unique_ptr<YGNode> clone {YGNodeClone(self.mContentYoga.get())};
    const auto yoga = clone.get();
    YGNodeStyleSetWidth(yoga, width);
    YGNodeCalculateLayout(yoga, YGUndefined, YGUndefined, YGDirectionLTR);
    const auto h = YGNodeLayoutGetHeight(yoga);
    return {std::ceil(width), std::ceil(h)};
  }

  return {std::ceil(width), std::ceil(height)};
}

void ScrollView::UpdateContentLayout() {
  const auto outer = mContentOuter->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(outer);
  const auto h = YGNodeLayoutGetHeight(outer);

  if (YGFloatIsUndefined(w) || YGFloatIsUndefined(h)) {
    mDirtyContentLayout = true;
    return;
  }

  const auto contentRoot = mContentYoga.get();
  if (!std::exchange(mDirtyContentLayout, false)) {
    const auto cw = YGNodeLayoutGetWidth(contentRoot);
    const auto ch = YGNodeLayoutGetHeight(contentRoot);
    if (std::abs(w - cw) > 1 || std::abs(h - ch) > 1) {
      YGNodeMarkDirty(mContentOuter->GetLayoutNode());
    }
    return;
  }

  YGNodeCalculateLayout(contentRoot, w, YGUndefined, YGDirectionLTR);

  this->UpdateScrollBars();
}

}// namespace FredEmmott::GUI::Widgets