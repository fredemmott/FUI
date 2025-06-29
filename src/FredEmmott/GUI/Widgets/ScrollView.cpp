// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <Windows.h>

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>

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
    mContent.reset(new Widget(2, {ContentStyleClass}));
  });
  YGNodeRemoveAllChildren(this->GetLayoutNode());
  YGNodeSetMeasureFunc(this->GetLayoutNode(), &ScrollView::Measure);
  {
    const auto child = mContent->GetLayoutNode();
    mContentYoga.reset(YGNodeNew());
    YGNodeStyleSetOverflow(mContentYoga.get(), YGOverflowScroll);
    YGNodeSetChildren(mContentYoga.get(), &child, 1);

    YGNodeSetContext(
      mContentYoga.get(),
      new widget_detail::YogaContext {
        widget_detail::DetachedYogaTree {
          .mLogicalParent = this,
          .mFosterParent = mContent.get(),
        },
      });
  }
  {
    mScrollBarsYoga.reset(YGNodeNew());
    const std::array scrollbars {
      mHorizontalScrollBar->GetLayoutNode(),
      mVerticalScrollBar->GetLayoutNode(),
    };
    YGNodeSetChildren(
      mScrollBarsYoga.get(), scrollbars.data(), scrollbars.size());
  }

  using StaticTheme::ScrollBar::ScrollBarSize;

  constexpr auto SmoothScrollingAnimation = CubicBezierStyleTransition(
    std::chrono::milliseconds(100),
    StaticTheme::Common::ControlFastOutSlowInKeySpline);
  mContent->SetBuiltInStyles({
    .mAlignSelf = YGAlignStretch,
    .mFlexDirection = YGFlexDirectionColumn,
    .mTranslateX = {0, SmoothScrollingAnimation},
    .mTranslateY = {0, SmoothScrollingAnimation},
  });
  mVerticalScrollBar->SetAdditionalBuiltInStyles({
    .mBottom = ScrollBarSize,
    .mPosition = YGPositionTypeAbsolute,
    .mRight = 4,
    .mTop = 0,
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
  const auto ctx = static_cast<widget_detail::YogaContext*>(
    YGNodeGetContext(mContentYoga.get()));
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
    mContent.get(),
  };
}

Widget* ScrollView::GetFosterParent() const noexcept {
  return mContent.get();
}

void ScrollView::UpdateLayout() {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);
  YGNodeCalculateLayout(mContentYoga.get(), w, YGUndefined, YGDirectionLTR);
  YGNodeCalculateLayout(mScrollBarsYoga.get(), w, h, YGDirectionLTR);

  const auto cw = YGNodeLayoutGetWidth(mContentYoga.get());
  const auto ch = YGNodeLayoutGetHeight(mContentYoga.get());

  const bool showHScroll
    = IsScrollBarVisible(mHorizontalScrollBarVisibility, cw, w);
  const bool showVScroll
    = IsScrollBarVisible(mVerticalScrollBarVisibility, ch, h);
  mHorizontalScrollBar->AddExplicitStyles(
    Style {
      .mDisplay = showHScroll ? YGDisplayFlex : YGDisplayNone,
    });
  mVerticalScrollBar->AddExplicitStyles(
    Style {
      .mDisplay = showVScroll ? YGDisplayFlex : YGDisplayNone,
    });

  if (showHScroll) {
    mHorizontalScrollBar->SetThumbSize(w);
    mHorizontalScrollBar->SetMaximum(cw - w);
  } else {
    mHorizontalScrollBar->SetValue(0);
    mHorizontalScrollBar->SetMaximum(0);
  }

  if (showVScroll) {
    mVerticalScrollBar->SetThumbSize(h);
    mVerticalScrollBar->SetMaximum(ch - h);
  } else {
    mVerticalScrollBar->SetValue(0);
    mVerticalScrollBar->SetMaximum(0);
  }

  Widget::UpdateLayout();
}

void ScrollView::PaintChildren(Renderer* renderer) const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);
  YGNodeCalculateLayout(mContentYoga.get(), w, h, YGDirectionLTR);
  YGNodeCalculateLayout(mScrollBarsYoga.get(), w, h, YGDirectionLTR);

  {
    const auto clipTo = renderer->ScopedClipRect({.mSize = {w, h}});
    mContent->Paint(renderer);
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

void ScrollView::OnHorizontalScroll(float value) {
  mContent->AddExplicitStyles({.mTranslateX = -value});
}

void ScrollView::OnVerticalScroll(float value) {
  mContent->AddExplicitStyles({.mTranslateY = -value});
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
YGSize ScrollView::Measure(
  YGNodeConstRef node,
  float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  float height,
  [[maybe_unused]] YGMeasureMode heightMode) {
  const auto& self = *static_cast<ScrollView*>(FromYogaNode(node));

  const auto root = self.mContentYoga.get();
  if (height == 0) {
    height = YGUndefined;
  }
  YGNodeCalculateLayout(root, width, height, YGDirectionLTR);
  width = YGNodeLayoutGetWidth(root);
  height = YGNodeLayoutGetHeight(root);
  return {width, height};
}

}// namespace FredEmmott::GUI::Widgets