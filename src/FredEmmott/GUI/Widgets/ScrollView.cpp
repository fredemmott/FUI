// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>
#include <print>

#include "FredEmmott/GUI/SystemSettings.hpp"
#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "FredEmmott/utility/almost_equal.hpp"

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
  YGNodeSetMeasureFunc(
    mContentOuter->GetLayoutNode(), &ScrollView::MeasureOuterContent);
  mContentOuter->SetContextIfUnset<ScrollViewContext>(this);

  using StaticTheme::ScrollBar::ScrollBarSize;

  constexpr auto SmoothScrollingAnimation = CubicBezierStyleTransition(
    std::chrono::milliseconds(100),
    StaticTheme::Common::ControlFastOutSlowInKeySpline);
  mContentInner->SetBuiltInStyles(
    Style()
      .TranslateX(0, SmoothScrollingAnimation)
      .TranslateY(0, SmoothScrollingAnimation));
  mVerticalScrollBar->SetAdditionalBuiltInStyles(
    Style().Position(YGPositionTypeAbsolute).Right(4).Top(4));
  mHorizontalScrollBar->SetAdditionalBuiltInStyles(
    Style()
      .Bottom(4)
      .FlexGrow(1)
      .Left(0.f)
      .Position(YGPositionTypeAbsolute)
      .Right(ScrollBarSize));

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

void ScrollView::UpdateScrollBars(const Size& containerSize) const {
  const auto [w, h] = containerSize;

  if (YGFloatIsUndefined(w) || YGFloatIsUndefined(h)) {
    return;
  }

  const auto cw = YGNodeLayoutGetWidth(mContentYoga.get());
  const auto ch = YGNodeLayoutGetHeight(mContentYoga.get());

  const bool showHScroll
    = IsScrollBarVisible(mHorizontalScrollBarVisibility, cw, w);
  const bool showVScroll
    = IsScrollBarVisible(mVerticalScrollBarVisibility, ch, h);

  if (showHScroll) {
    mHorizontalScrollBar->SetThumbSize(w);
    mHorizontalScrollBar->SetMaximum(cw - w);
    mHorizontalScrollBar->AddExplicitStyles(Style().Display(YGDisplayFlex));
    mVerticalScrollBar->AddExplicitStyles(
      Style().Bottom(StaticTheme::ScrollBar::ScrollBarSize + 4));
  } else {
    mHorizontalScrollBar->SetValue(0);
    mHorizontalScrollBar->SetMaximum(0);
    mHorizontalScrollBar->AddExplicitStyles(Style().Display(YGDisplayNone));
    mVerticalScrollBar->AddExplicitStyles(Style().Bottom(4));
  }

  if (showVScroll) {
    mVerticalScrollBar->SetThumbSize(h);
    mVerticalScrollBar->SetMaximum(ch - h);
    mVerticalScrollBar->AddExplicitStyles(Style().Display(YGDisplayFlex));
  } else {
    mVerticalScrollBar->SetValue(0);
    mVerticalScrollBar->SetMaximum(0);
    mVerticalScrollBar->AddExplicitStyles(Style().Display(YGDisplayNone));
  }
}

void ScrollView::PaintChildren(Renderer* renderer) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  {
    const auto clipTo = renderer->ScopedClipRect(Size {w, h});
    const auto contentWidth = std::floor(w);
    if (!utility::almost_equal(
          contentWidth, YGNodeLayoutGetWidth(mContentYoga.get()))) {
      YGNodeCalculateLayout(
        mContentYoga.get(), contentWidth, YGUndefined, YGDirectionLTR);
    }
    UpdateScrollBars({w, h});
    mContentInner->Paint(renderer);
  }

  mHorizontalScrollBar->Paint(renderer);
  mVerticalScrollBar->Paint(renderer);
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
  return Style().MinHeight(128).MinWidth(128);
}

void ScrollView::OnHorizontalScroll(
  const float value,
  const ScrollBar::ChangeReason reason) {
  std::optional<StyleTransition> transition;
  if (reason != ScrollBar::ChangeReason::Discrete) {
    transition = InstantStyleTransition;
  }
  mContentInner->AddExplicitStyles(Style().TranslateX(-value, transition));
}

void ScrollView::OnVerticalScroll(float value, ScrollBar::ChangeReason reason) {
  std::optional<StyleTransition> transition;
  if (reason != ScrollBar::ChangeReason::Discrete) {
    transition = InstantStyleTransition;
  }
  mContentInner->AddExplicitStyles(Style().TranslateY(-value, transition));
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

  const bool fits
    = content < container || utility::almost_equal(content, container);

  return !fits;
}

void ScrollView::OnInnerContentDirty(YGNodeConstRef node) {
  auto& self
    = *FromYogaNode(node)->GetContext<ScrollViewContext>()->mScrollView;
  self.mDirtyInner = true;
  YGNodeMarkDirty(self.mContentOuter->GetLayoutNode());
}

YGSize ScrollView::MeasureOuterContent(
  YGNodeConstRef node,
  float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto outer = FromYogaNode(node);
  auto& self = *outer->GetContext<ScrollViewContext>()->mScrollView;

  const bool haveDirtyInner = std::exchange(self.mDirtyInner, false);
  const bool haveWidthChange
    = (!std::isnan(width) && std::abs(width - YGNodeLayoutGetWidth(self.GetLayoutNode())) >= 1);
  const bool haveHeightChange
    = (!std::isnan(height) && std::abs(height - YGNodeLayoutGetHeight(self.GetLayoutNode())) >= 1);
  const bool haveSizeChange = haveWidthChange || haveHeightChange;

  const auto yoga = self.mContentYoga.get();

  float contentWidth = YGNodeLayoutGetWidth(yoga);
  float contentHeight = YGNodeLayoutGetHeight(yoga);
  if (haveDirtyInner || haveSizeChange) {
    contentWidth = self.mContentInnerMinWidth
      = (haveDirtyInner ? GetMinimumWidth(yoga) : self.mContentInnerMinWidth);
    contentHeight = GetIdealHeight(yoga, contentWidth);
  }

  width = contentWidth;
  using enum CSSMeasureMode;
  switch (static_cast<CSSMeasureMode>(heightMode)) {
    case StretchFit:
      // Should always use provided height
      break;
    case FitContent:
      height = std::min(height, contentHeight);
      break;
    case MaxContent:
      height = contentHeight;
      break;
  };

  YGNodeCalculateLayout(yoga, contentWidth, YGUndefined, YGDirectionLTR);
  return {std::ceil(width), std::ceil(height)};
}

}// namespace FredEmmott::GUI::Widgets