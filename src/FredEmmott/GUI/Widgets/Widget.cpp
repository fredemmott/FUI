// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <core/SkRRect.h>

#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

namespace {

struct MouseCapture {
  Widget* mWidget {nullptr};
  SkPoint mOffset {};
};
std::optional<MouseCapture> gMouseCapture;

void PaintBackground(SkCanvas* canvas, const SkRect& rect, const Style& style) {
  if (!style.mBackgroundColor) {
    return;
  }

  auto paint = style.mBackgroundColor->GetPaint(rect);

  if (!style.mBorderRadius) {
    canvas->drawRect(rect, paint);
    return;
  }

  auto rrect = rect;
  auto radius = style.mBorderRadius.value();
  if (style.mBorderWidth && style.mBorderColor) {
    radius -= style.mBorderWidth.value();
    const auto inset = style.mBorderWidth.value() - 0.5f;
    rrect.inset(inset, inset);
  }

  paint.setAntiAlias(true);
  canvas->drawRoundRect(rrect, radius, radius, paint);
}

void PaintBorder(
  YGNodeConstRef yoga,
  SkCanvas* canvas,
  const SkRect& contentRect,
  const Style& style) {
  if (!(style.mBorderColor)) {
    return;
  }

  const auto top = YGNodeLayoutGetBorder(yoga, YGEdgeTop);
  const auto left = YGNodeLayoutGetBorder(yoga, YGEdgeLeft);
  const auto bottom = YGNodeLayoutGetBorder(yoga, YGEdgeBottom);
  const auto right = YGNodeLayoutGetBorder(yoga, YGEdgeRight);

  if (top != left || top != right || top != bottom) {
    throw std::logic_error(
      "Only equal-thickness borders are currently supported");
  }
  if (top == 0) {
    return;
  }
  const auto borderRect = contentRect.makeInset(top / 2.0, top / 2.0);

  auto paint = style.mBorderColor->GetPaint(contentRect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(top);

  if (!style.mBorderRadius) {
    canvas->drawRect(borderRect, paint);
    return;
  }

  auto radius = style.mBorderRadius.value();
  paint.setAntiAlias(true);
  canvas->drawRoundRect(borderRect, radius, radius, paint);
}
}// namespace

Widget::Widget(std::size_t id, const StyleClasses& classes)
  : mClassList(classes),
    mID(id),
    mYoga(YGNodeNewWithConfig(GetYogaConfig())) {
  YGNodeSetContext(mYoga.get(), this);
  mStyleTransitions.reset(new StyleTransitions());
}

WidgetList Widget::GetDirectChildren() const noexcept {
  return WidgetList {mManagedChildrenCacheForGetChildren};
}

void Widget::ChangeDirectChildren(const std::function<void()>& mutator) {
  const auto layout = this->GetLayoutNode();
  YGNodeRemoveAllChildren(layout);

  if (mutator) {
    mutator();
  }

  const auto childLayouts
    = std::views::transform(this->GetDirectChildren(), &Widget::GetLayoutNode)
    | std::ranges::to<std::vector>();
  YGNodeSetChildren(layout, childLayouts.data(), childLayouts.size());
}

Widget::~Widget() {
  this->EndMouseCapture();
}

bool Widget::IsDisabled() const {
  return ((mDirectStateFlags | mInheritedStateFlags) & StateFlags::Disabled)
    != StateFlags::Default;
}

FrameRateRequirement Widget::GetFrameRateRequirement() const noexcept {
  using enum StateFlags;
  if ((mDirectStateFlags & Animating) != StateFlags::Default) {
    return FrameRateRequirement::SmoothAnimation;
  }
  for (auto&& child: this->GetDirectChildren()) {
    if (
      child->GetFrameRateRequirement()
      == FrameRateRequirement::SmoothAnimation) {
      return FrameRateRequirement::SmoothAnimation;
    }
  }
  return FrameRateRequirement::None;
}

bool Widget::IsDirectlyDisabled() const {
  return (mDirectStateFlags & StateFlags::Disabled) != StateFlags::Default;
}

void Widget::SetIsDirectlyDisabled(bool value) {
  if (value) {
    mDirectStateFlags |= StateFlags::Disabled;
  } else {
    mDirectStateFlags &= ~StateFlags::Disabled;
  }
}

void Widget::ReplaceExplicitStyles(const Style& styles) {
  if (styles == mExplicitStyles) {
    return;
  }
  mExplicitStyles = styles;

  this->ComputeStyles(mInheritedStyles);
}

void Widget::AddExplicitStyles(const Style& styles) {
  mExplicitStyles += styles;
  this->ComputeStyles(mInheritedStyles);
}

void Widget::SetBuiltInStyles(const Style& styles) {
  mReplacedBuiltInStyles = styles;
}
void Widget::SetAdditionalBuiltInStyles(const Style& styles) {
  mReplacedBuiltInStyles = this->GetBuiltInStyles() + styles;
}

void Widget::SetManagedChildren(const std::vector<Widget*>& children) {
  std::vector<unique_ptr<Widget>> newChildren;
  for (auto child: children) {
    auto it
      = std::ranges::find(mManagedChildren, child, &unique_ptr<Widget>::get);
    if (it == mManagedChildren.end()) {
      newChildren.emplace_back(child);
    } else {
      newChildren.emplace_back(std::move(*it));
    }
  }
  mManagedChildren = std::move(newChildren);
  mManagedChildrenCacheForGetChildren = children;
}

void Widget::Paint(SkCanvas* canvas) const {
  const auto& style = mComputedStyle;

  if (style.mDisplay == YGDisplayNone) {
    return;
  }

  const auto opacity = style.mOpacity.value_or_default();
  if (opacity <= std::numeric_limits<float>::epsilon()) {
    return;
  }
  if (opacity + std::numeric_limits<float>::epsilon() >= 1.0f) {
    canvas->save();
  } else {
    canvas->saveLayerAlphaf(nullptr, opacity);
  }
  struct scope_exit_t {
    SkCanvas* mCanvas {nullptr};
    ~scope_exit_t() {
      mCanvas->restore();
    }
  };
  const scope_exit_t restore {canvas};

  const auto yoga = this->GetLayoutNode();
  canvas->translate(
    YGNodeLayoutGetLeft(yoga) + style.mTranslateX.value_or_default()
      - mScrollOffset.fX,
    YGNodeLayoutGetTop(yoga) + style.mTranslateY.value_or_default()
      - mScrollOffset.fY);
  auto rect
    = SkRect::MakeWH(YGNodeLayoutGetWidth(yoga), YGNodeLayoutGetHeight(yoga));

  const auto scaleY = style.mScaleY.value_or_default();
  if (
    scaleY + std::numeric_limits<float>::epsilon() < 1.0f
    || scaleY > 1.0f + std::numeric_limits<float>::epsilon()) {
    canvas->scale(1.0f, scaleY);
    canvas->translate(0, (rect.height() * (1.0f - scaleY) / 2.0f));
    rect = SkRect::MakeWH(rect.width(), rect.height() * scaleY);
  }

  PaintBackground(canvas, rect, style);
  PaintBorder(yoga, canvas, rect, style);
  this->PaintOwnContent(canvas, rect, style);
  this->PaintChildren(canvas);
}

void Widget::PaintChildren(SkCanvas* canvas) const {
  const auto children = this->GetDirectChildren();
  if (children.empty()) {
    return;
  }

  for (auto&& child: children) {
    child->Paint(canvas);
  }
}

void Widget::SetChildren(const std::vector<Widget*>& children) {
  if (children == mManagedChildrenCacheForGetChildren) {
    return;
  }

  const auto foster = this->GetFosterParent();
  const auto parent = foster ? foster : this;

  parent->ChangeDirectChildren(
    std::bind_front(&Widget::SetManagedChildren, parent, std::ref(children)));
}
void Widget::DispatchEvent(const Event* e) {
  if (const auto it = dynamic_cast<const MouseEvent*>(e)) {
    if (gMouseCapture) {
      auto translated = it->WithOffset(gMouseCapture->mOffset);
      (void)gMouseCapture->mWidget->DispatchMouseEvent(translated);
    } else {
      (void)this->DispatchMouseEvent(*it);
    }
    return;
  }
  // whut?
  __debugbreak();
}

void Widget::Tick() {
  for (auto&& child: this->GetDirectChildren()) {
    child->Tick();
  }
}

void Widget::UpdateLayout() {
  for (auto&& child: this->GetDirectChildren()) {
    child->UpdateLayout();
  }
}

Widget::EventHandlerResult Widget::DispatchMouseEvent(
  const MouseEvent& parentEvent) {
  auto event = parentEvent;

  const auto layout = this->GetLayoutNode();
  const auto display = YGNodeStyleGetDisplay(layout);
  if (display != YGDisplayContents) {
    event = event.WithOffset({
      mScrollOffset.fX - YGNodeLayoutGetLeft(layout),
      mScrollOffset.fY - YGNodeLayoutGetTop(layout),
    });
  }
  mMouseOffset = event.mOffset;

  const auto w = YGNodeLayoutGetWidth(layout);
  const auto h = YGNodeLayoutGetHeight(layout);

  const auto [x, y] = event.GetPosition();

  if (
    (x < 0 || y < 0 || x > w || y > h)
    && !(gMouseCapture && gMouseCapture->mWidget == this)) {
    mDirectStateFlags &= ~StateFlags::Hovered;

    if (display != YGDisplayContents) {
      event.mWindowPoint
        = {SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity};
      event.mOffset = {};
    }
  } else {
    mDirectStateFlags |= StateFlags::Hovered;
  }

  // Always propagate unconditionally to allow correct internal states
  auto result = EventHandlerResult::Default;
  for (auto&& child: this->GetDirectChildren()) {
    if (YGNodeStyleGetDisplay(child->GetLayoutNode()) == YGDisplayNone) {
      continue;
    }
    if (
      child->DispatchMouseEvent(event) == EventHandlerResult::StopPropagation) {
      result = EventHandlerResult::StopPropagation;
    }
  }

  if (result == EventHandlerResult::StopPropagation) {
    return result;
  }

  if (std::holds_alternative<MouseEvent::ButtonPressEvent>(event.mDetail)) {
    result = this->OnMouseButtonPress(event);
  } else if (std::holds_alternative<MouseEvent::ButtonReleaseEvent>(
               event.mDetail)) {
    result = this->OnMouseButtonRelease(event);
  } else {
    result = this->OnMouseMove(event);
  }

  return result;
}

Widget::EventHandlerResult Widget::OnMouseMove(const MouseEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseButtonPress(const MouseEvent& event) {
  if (this->IsDisabled()) {
    return EventHandlerResult::Default;
  }
  if ((mDirectStateFlags & StateFlags::Hovered) == StateFlags::Hovered) {
    mDirectStateFlags |= StateFlags::Active;
  } else {
    mDirectStateFlags &= ~StateFlags::Active;
  }
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseButtonRelease(
  const MouseEvent& event) {
  if (this->IsDisabled()) {
    return EventHandlerResult::Default;
  }
  constexpr auto Flags = StateFlags::Hovered | StateFlags::Active;
  const auto isClick = (mDirectStateFlags & Flags) == Flags;
  mDirectStateFlags &= ~StateFlags::Active;
  if (isClick) {
    return this->OnClick(event);
  }

  return EventHandlerResult::Default;
}

void Widget::StartMouseCapture() {
  gMouseCapture = MouseCapture {this, mMouseOffset};
}

void Widget::EndMouseCapture() {
  if (!gMouseCapture) {
    return;
  }
  if (gMouseCapture->mWidget != this) {
    return;
  }
  gMouseCapture = {};
}

void Widget::ScrollTo(const SkPoint& offset) {
  mScrollOffset = offset;
}

}// namespace FredEmmott::GUI::Widgets