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

namespace {
const auto ScrollViewStyleClass = StyleClass::Make("ScrollView");
const auto ContentStyleClass = StyleClass::Make("ScrollView_Content");
}// namespace

ScrollView::ScrollView(std::size_t id, const StyleClasses& classes)
  : Widget(id, classes + ScrollViewStyleClass) {
  this->ChangeDirectChildren([this] {
    mHorizontalScrollBar.reset(new ScrollBar(0, Orientation::Horizontal));
    mVerticalScrollBar.reset(new ScrollBar(1, Orientation::Vertical));
    mContentOuter.reset(new Widget(2, {ContentStyleClass}));
  });
  mContentOuter->SetChildren({mContentInner = new Widget(0)});
  YGNodeStyleSetOverflow(this->GetLayoutNode(), YGOverflowScroll);
  using StaticTheme::ScrollBar::ScrollBarSize;

  constexpr auto SmoothScrollingAnimation = CubicBezierStyleTransition(
    std::chrono::milliseconds(100),
    StaticTheme::Common::ControlFastOutSlowInKeySpline);
  mContentOuter->SetBuiltInStyles({
    .mAlignSelf = YGAlignStretch,
    .mFlexDirection = YGFlexDirectionColumn,
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

ScrollView::~ScrollView() {}

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
  };
}

Widget* ScrollView::GetFosterParent() const noexcept {
  return mContentInner;
}

void ScrollView::UpdateLayout() {
  const auto node = this->GetLayoutNode();
  mContentOuter->UpdateLayout();
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

  Widget::UpdateLayout();
}

void ScrollView::PaintChildren(Renderer* renderer) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  {
    const auto clipTo = renderer->ScopedClipRect(Size {w, h});
    mContentOuter->Paint(renderer);
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
  return Style {
    .mFlexShrink = 1,
  };
}

void ScrollView::OnHorizontalScroll(float value) {
  mContentOuter->AddExplicitStyles({.mTranslateX = -value});
}

void ScrollView::OnVerticalScroll(float value) {
  mContentOuter->AddExplicitStyles({.mTranslateY = -value});
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

}// namespace FredEmmott::GUI::Widgets