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
constexpr LiteralStyleClass ScrollViewStyleClass("ScrollView");
constexpr LiteralStyleClass ContentOuterStyleClass("ScrollView/ContentOuter");
constexpr LiteralStyleClass ContentInnerStyleClass("ScrollView/ContentInner");

constexpr auto SmoothScrollingAnimation = CubicBezierStyleTransition(
  std::chrono::milliseconds(100),
  StaticTheme::Common::ControlFastOutSlowInKeySpline);

auto& ScrollViewStyle() {
  static const ImmutableStyle ret {
    Style().MinHeight(128).MinWidth(128).FlexGrow(1).AlignSelf(Align::Stretch),
  };
  return ret;
}

auto& ContentInnerStyle() {
  static const ImmutableStyle ret {
    Style()
      .TranslateX(0, SmoothScrollingAnimation)
      .TranslateY(0, SmoothScrollingAnimation),
  };
  return ret;
}

auto& VerticalScrollBarStyle() {
  static const auto ret = ScrollBar::MakeImmutableStyle(
    Orientation::Vertical,
    Style().Position(PositionType::Absolute).Right(4).Top(4));
  return ret;
}

auto& HorizontalScrollBarStyle() {
  using StaticTheme::ScrollBar::ScrollBarSize;
  static const auto ret = ScrollBar::MakeImmutableStyle(
    Orientation::Horizontal,
    Style()
      .Bottom(4)
      .FlexGrow(1)
      .Left(0.f)
      .Position(PositionType::Absolute)
      .Right(ScrollBarSize));
  return ret;
}

}// namespace

ScrollView::ScrollView(Window* const window, const StyleClasses& classes)
  : Widget(window, ScrollViewStyleClass, ScrollViewStyle(), classes) {
  this->SetStructuralChildren({
    mHorizontalScrollBar = new ScrollBar(
      window, HorizontalScrollBarStyle(), Orientation::Horizontal),
    mVerticalScrollBar
    = new ScrollBar(window, VerticalScrollBarStyle(), Orientation::Vertical),
    mContentOuter = new Widget(window, ContentOuterStyleClass, {}),
    mContentInner = new Widget(
      window,
      ContentInnerStyleClass,
      ContentInnerStyle(),
      {PseudoClasses::LayoutOrphan}),
  });
  this->SetStructuralParentForLogicalChildren(mContentInner);

  mContentYoga.reset(YGNodeNew());
  YGNodeInsertChild(mContentYoga.get(), mContentInner->GetLayoutNode(), 0);

  YGNodeSetDirtiedFunc(
    mContentInner->GetLayoutNode(), &ScrollView::OnInnerContentDirty);
  YGNodeSetMeasureFunc(
    mContentOuter->GetLayoutNode(), &ScrollView::MeasureOuterContent);

  mHorizontalScrollBar->OnValueChanged(
    std::bind_front(&ScrollView::OnHorizontalScroll, this));
  mVerticalScrollBar->OnValueChanged(
    std::bind_front(&ScrollView::OnVerticalScroll, this));
}

ScrollView::~ScrollView() = default;

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

  const auto oldHValue = mHorizontalScrollBar->GetValue();
  const auto oldVValue = mVerticalScrollBar->GetValue();

  if (showHScroll) {
    mHorizontalScrollBar->SetThumbSize(w);
    mHorizontalScrollBar->SetRange(0, cw - w);
    mHorizontalScrollBar->AddMutableStyles(Style().Display(Display::Flex));
    mVerticalScrollBar->AddMutableStyles(
      Style().Bottom(StaticTheme::ScrollBar::ScrollBarSize + 4));
  } else {
    mHorizontalScrollBar->SetValue(0);
    mHorizontalScrollBar->SetRange(0, 0);
    mHorizontalScrollBar->AddMutableStyles(Style().Display(Display::None));
    mVerticalScrollBar->AddMutableStyles(Style().Bottom(4));
  }

  if (showVScroll) {
    mVerticalScrollBar->SetThumbSize(h);
    mVerticalScrollBar->SetRange(0, ch - h);
    mVerticalScrollBar->AddMutableStyles(Style().Display(Display::Flex));
  } else {
    mVerticalScrollBar->SetValue(0);
    mVerticalScrollBar->SetRange(0, 0);
    mVerticalScrollBar->AddMutableStyles(Style().Display(Display::None));
  }

  const auto inheritable = GetComputedStyle().InheritableValues();
  mHorizontalScrollBar->ComputeStyles(inheritable);
  mVerticalScrollBar->ComputeStyles(inheritable);
  FUI_ASSERT(
    mHorizontalScrollBar->GetComputedStyle().Display()
    == (showHScroll ? Display::Flex : Display::None));
  FUI_ASSERT(
    mVerticalScrollBar->GetComputedStyle().Display()
    == (showVScroll ? Display::Flex : Display::None));

  const auto newHValue = mHorizontalScrollBar->GetValue();
  const auto newVValue = mVerticalScrollBar->GetValue();
  if (
    utility::almost_equal(oldHValue, newHValue)
    && utility::almost_equal(oldVValue, newVValue)) {
    return;
  }

  mContentInner->SetMutableStyles(
    Style()
      .TranslateX(-mHorizontalScrollBar->GetValue(), InstantStyleTransition)
      .TranslateY(-mVerticalScrollBar->GetValue(), InstantStyleTransition));
  mContentInner->ComputeStyles(inheritable);
}

void ScrollView::PaintChildren(Renderer* renderer) const {
  const auto node = this->GetLayoutNode();
  if (YGNodeGetHasNewLayout(node)) {
    this->UpdateLayout();
    YGNodeSetHasNewLayout(node, false);
  }
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);
  const auto clipTo = renderer->ScopedClipRect(Size {w, h});

  mContentInner->Paint(renderer);
  mHorizontalScrollBar->Paint(renderer);
  mVerticalScrollBar->Paint(renderer);
}

Widget::EventHandlerResult ScrollView::OnMouseVerticalWheel(
  const MouseEvent& e) {
  const auto delta = std::get<MouseEvent::VerticalWheelEvent>(e.mDetail).mDelta;
  const auto lines = delta * SystemSettings::Get().GetMouseWheelScrollLines();
  const auto pixels
    = lines * SystemFont::Resolve(SystemFont::Body).GetMetrics().mSize;

  const auto scrollBar = mVerticalScrollBar;
  scrollBar->SetValue(scrollBar->GetValue() + pixels);
  return EventHandlerResult::StopPropagation;
}

void ScrollView::UpdateLayout() const {
  const auto node = this->GetLayoutNode();
  const auto w = YGNodeLayoutGetWidth(node);
  const auto h = YGNodeLayoutGetHeight(node);

  const auto contentWidth = std::floor(w);
  if (!utility::almost_equal(
        contentWidth, YGNodeLayoutGetWidth(mContentYoga.get()))) {
    // This should be a no-op, but the YGNodeCalculateLayout call can end up
    // re-using the cached layout inappropriately without this.
    YGNodeStyleSetWidth(mContentYoga.get(), contentWidth);
  }
  YGNodeCalculateLayout(
    mContentYoga.get(), contentWidth, YGUndefined, YGDirectionLTR);
  UpdateScrollBars({w, h});
}

void ScrollView::OnHorizontalScroll(
  const float value,
  const ScrollBar::ChangeReason reason) {
  std::optional<StyleTransition> transition;
  if (reason != ScrollBar::ChangeReason::Discrete) {
    transition = InstantStyleTransition;
  }
  mContentInner->AddMutableStyles(Style().TranslateX(-value, transition));
}

void ScrollView::OnVerticalScroll(float value, ScrollBar::ChangeReason reason) {
  std::optional<StyleTransition> transition;
  if (reason != ScrollBar::ChangeReason::Discrete) {
    transition = InstantStyleTransition;
  }
  mContentInner->AddMutableStyles(Style().TranslateY(-value, transition));
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

void ScrollView::OnInnerContentDirty(const YGNode* node) {
  auto& self = *FromYogaNode(node)->GetStructuralParent<ScrollView>();
  self.mDirtyInner = true;
  YGNodeMarkDirty(self.mContentOuter->GetLayoutNode());
}

YGSize ScrollView::MeasureOuterContent(
  const YGNode* node,
  float width,
  [[maybe_unused]] YGMeasureMode widthMode,
  float height,
  YGMeasureMode heightMode) {
  const auto outer = FromYogaNode(node);
  auto& self = *outer->GetLogicalParent<ScrollView>();

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