// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

#include "FredEmmott/GUI/assert.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

namespace {

struct MouseCapture {
  Widget* mWidget {nullptr};
  Point mOffset {};
};
std::optional<MouseCapture> gMouseCapture;

void PaintBackground(Renderer* renderer, const Rect& rect, const Style& style) {
  if (!style.BackgroundColor()) {
    return;
  }

  auto brush = *style.BackgroundColor();

  const auto baseRadius = style.BorderRadius().value_or(0);
  const auto tlRadius = style.BorderTopLeftRadius().value_or(baseRadius);
  const auto trRadius = style.BorderTopRightRadius().value_or(baseRadius);
  const auto brRadius = style.BorderBottomRightRadius().value_or(baseRadius);
  const auto blRadius = style.BorderBottomLeftRadius().value_or(baseRadius);

  constexpr auto Eps = std::numeric_limits<float>::epsilon();

  if (tlRadius < Eps && trRadius < Eps && brRadius < Eps && blRadius < Eps) {
    renderer->FillRect(brush, rect);
    return;
  }
  const auto allSameRadii = (tlRadius == trRadius) && (trRadius == brRadius)
    && (brRadius == blRadius);

  if (allSameRadii) {
    renderer->FillRoundedRect(brush, rect, baseRadius);
    return;
  }

  renderer->FillRoundedRect(
    brush, rect, tlRadius, trRadius, brRadius, blRadius);
}

void PaintBorder(
  YGNodeConstRef yoga,
  Renderer* renderer,
  const Rect& contentRect,
  const Style& style) {
  if (!style.BorderColor()) {
    return;
  }

  const auto top = YGNodeLayoutGetBorder(yoga, YGEdgeTop);
  const auto left = YGNodeLayoutGetBorder(yoga, YGEdgeLeft);
  const auto bottom = YGNodeLayoutGetBorder(yoga, YGEdgeBottom);
  const auto right = YGNodeLayoutGetBorder(yoga, YGEdgeRight);

  constexpr auto Eps = std::numeric_limits<float>::epsilon();
  if (top < Eps && left < Eps && right < Eps && bottom < Eps) {
    return;
  }

  const auto allSame = (top == left) && (top == right) && (top == bottom);
  const auto borderRect
    = contentRect.WithInset(left / 2, top / 2, right / 2, bottom / 2);

  const auto brush = *style.BorderColor();

  if (
    style.BorderRadius().value_or(0) < std::numeric_limits<float>::epsilon()) {
    if (allSame) {
      renderer->StrokeRect(brush, borderRect);
      return;
    }
    if (top > Eps) {
      renderer->DrawLine(
        brush, contentRect.GetTopLeft(), contentRect.GetTopRight(), top);
    }
    if (right > Eps) {
      renderer->DrawLine(
        brush, contentRect.GetTopRight(), contentRect.GetBottomRight(), right);
    }
    if (bottom > Eps) {
      renderer->DrawLine(
        brush,
        contentRect.GetBottomRight(),
        contentRect.GetBottomLeft(),
        bottom);
    }
    if (left > Eps) {
      renderer->DrawLine(
        brush, contentRect.GetBottomLeft(), contentRect.GetTopLeft(), left);
    }
    return;
  }

  if (!allSame) {
    throw std::logic_error(
      "Only equal-thickness borders are currently supported if mBorderRadius "
      "is set");
  }

  const auto radius = style.BorderRadius().value();
  renderer->StrokeRoundedRect(brush, borderRect, radius);
}
}// namespace

Widget::Widget(std::size_t id, const StyleClasses& classes)
  : mClassList(classes),
    mID(id),
    mYoga(YGNodeNewWithConfig(GetYogaConfig())) {
  YGNodeSetContext(mYoga.get(), new YogaContext {this});
  mStyleTransitions.reset(new StyleTransitions());
}

WidgetList Widget::GetDirectChildren() const noexcept {
  return WidgetList {mManagedChildrenCacheForGetChildren};
}

void Widget::ChangeDirectChildren(const std::function<void()>& mutator) {
  if (mutator) {
    mutator();
  }

  const auto children = this->GetDirectChildren();
  std::vector<YGNodeRef> layoutChildren;
  layoutChildren.reserve(children.end() - children.begin());
  for (auto&& child: children) {
    if (!child->mClassList.contains(PseudoClasses::LayoutOrphan)) {
      layoutChildren.push_back(child->GetLayoutNode());
      continue;
    }
    auto& ctx
      = *static_cast<YogaContext*>(YGNodeGetContext(child->GetLayoutNode()));
    if (holds_alternative<Widget*>(ctx)) {
      ctx = DetachedYogaTree {this, child};
    }
  }
  YGNodeSetChildren(mYoga.get(), layoutChildren.data(), layoutChildren.size());
}

Widget* Widget::FromYogaNode(YGNodeConstRef node) {
  const auto it
    = std::get_if<Widget*>(static_cast<YogaContext*>(YGNodeGetContext(node)));
  return it ? *it : nullptr;
}

Widget::~Widget() {
  this->EndMouseCapture();

  const auto yogaContext
    = static_cast<YogaContext*>(YGNodeGetContext(mYoga.get()));
  YGNodeSetContext(mYoga.get(), nullptr);
  delete yogaContext;
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
}

void Widget::AddExplicitStyles(const Style& styles) {
  mExplicitStyles += styles;
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

void Widget::Paint(Renderer* renderer) const {
  const auto& style = mComputedStyle;

  if (style.Display() == YGDisplayNone) {
    return;
  }

  const auto opacity = style.Opacity().value_or(1.f);
  if (opacity <= std::numeric_limits<float>::epsilon()) {
    return;
  }

  const auto layer = renderer->ScopedLayer(opacity);
  const auto yoga = this->GetLayoutNode();
  renderer->Translate(
    YGNodeLayoutGetLeft(yoga) + style.TranslateX().value_or(0),
    YGNodeLayoutGetTop(yoga) + style.TranslateY().value_or(0));
  Rect rect {Size {
    YGNodeLayoutGetWidth(yoga),
    YGNodeLayoutGetHeight(yoga),
  }};

  const auto scaleX = style.ScaleX().value_or(1.f);
  const auto scaleY = style.ScaleY().value_or(1.f);
  if (
    std::min(scaleX, scaleY) + std::numeric_limits<float>::epsilon() < 1.0f
    || std::max(scaleX, scaleY)
      > 1.0f + std::numeric_limits<float>::epsilon()) {
    const auto oldWidth = rect.GetWidth();
    const auto newWidth = oldWidth * scaleX;
    const auto oldHeight = rect.GetHeight();
    const auto newHeight = oldHeight * scaleY;
    renderer->Translate((oldWidth - newWidth) / 2, (oldHeight - newHeight) / 2);
    renderer->Scale(scaleX, scaleY);
    rect.mSize = {rect.GetWidth(), rect.GetHeight()};
  }

  PaintBackground(renderer, rect, style);
  this->PaintOwnContent(renderer, rect, style);
  this->PaintChildren(renderer);
  PaintBorder(yoga, renderer, rect, style);
}

void Widget::PaintChildren(Renderer* renderer) const {
  const auto children = this->GetDirectChildren();
  if (children.empty()) {
    return;
  }

  for (auto&& child: children) {
    child->Paint(renderer);
  }
}

void Widget::SetChildren(const std::vector<Widget*>& children) {
  const auto foster = this->GetFosterParent();
  const auto parent = foster ? foster : this;
  if (children == parent->mManagedChildrenCacheForGetChildren) {
    return;
  }

  parent->ChangeDirectChildren(
    std::bind_front(&Widget::SetManagedChildren, parent, std::ref(children)));
}

Widget* Widget::DispatchEvent(const Event* e) {
  if (const auto it = dynamic_cast<const MouseEvent*>(e)) [[likely]] {
    if (gMouseCapture) {
      auto translated = it->WithOffset(gMouseCapture->mOffset);
      return gMouseCapture->mWidget->DispatchMouseEvent(translated).mTarget;
    }

    return this->DispatchMouseEvent(*it).mTarget;
  }
  throw std::logic_error("Unhandled event type");
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

Widget::MouseEventResult Widget::DispatchMouseEvent(
  const MouseEvent& parentEvent) {
  if (GetComputedStyle().PointerEvents() == PointerEvents::None) {
    return {};
  }

  auto event = parentEvent;

  const auto layout = this->GetLayoutNode();
  const auto display = YGNodeStyleGetDisplay(layout);
  if (display != YGDisplayContents) {
    event = event.WithOffset({
      -YGNodeLayoutGetLeft(layout),
      -YGNodeLayoutGetTop(layout),
    });
  }

  const auto w = YGNodeLayoutGetWidth(layout);
  const auto h = YGNodeLayoutGetHeight(layout);

  const auto [x, y] = event.GetPosition();

  MouseEventResult result;
  if (
    (x < 0 || y < 0 || x > w || y > h)
    && !(gMouseCapture && gMouseCapture->mWidget == this)) {
    mDirectStateFlags &= ~StateFlags::Hovered;

    if (display != YGDisplayContents) {
      event.mWindowPoint = {
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
      };
      event.mOffset = {};
    }
  } else {
    result.mTarget = this;
    mDirectStateFlags |= StateFlags::Hovered;
    event = event.WithOffset({
      -mComputedStyle.TranslateX().value_or(0),
      -mComputedStyle.TranslateY().value_or(0),
    });
  }

  mMouseOffset = event.mOffset;

  // Always propagate unconditionally to allow correct internal states
  for (auto&& child: this->GetDirectChildren()) {
    if (YGNodeStyleGetDisplay(child->GetLayoutNode()) == YGDisplayNone) {
      continue;
    }
    const auto it = child->DispatchMouseEvent(event);
    if (it.mResult == EventHandlerResult::StopPropagation) {
      result = it;
      FUI_ASSERT(it.mTarget);
    } else if (it.mTarget) {
      result.mTarget = it.mTarget;
    }
  }

  if (result.mResult == EventHandlerResult::StopPropagation) {
    FUI_ASSERT(result.mTarget);
    return result;
  }

  if (std::holds_alternative<MouseEvent::ButtonPressEvent>(event.mDetail)) {
    result.mResult = this->OnMouseButtonPress(event);
  } else if (std::holds_alternative<MouseEvent::ButtonReleaseEvent>(
               event.mDetail)) {
    result.mResult = this->OnMouseButtonRelease(event);
  } else if (std::holds_alternative<MouseEvent::MoveEvent>(event.mDetail)) {
    result.mResult = this->OnMouseMove(event);
  } else if (std::holds_alternative<MouseEvent::HorizontalWheelEvent>(
               event.mDetail)) {
    result.mResult = this->OnMouseHorizontalWheel(event);
  } else if (std::holds_alternative<MouseEvent::VerticalWheelEvent>(
               event.mDetail)) {
    result.mResult = this->OnMouseVerticalWheel(event);
#ifndef NDEBUG
  } else {
    OutputDebugStringA("Unhandled mouse event type\n");
    __debugbreak();
#endif
  }

  if (
    result.mResult == EventHandlerResult::StopPropagation && !result.mTarget) {
    result.mTarget = this;
  }

  FUI_ASSERT(
    result.mTarget
    || (mDirectStateFlags & StateFlags::Hovered) != StateFlags::Hovered);

  return result;
}

Widget::EventHandlerResult Widget::OnMouseMove(const MouseEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseVerticalWheel(const MouseEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseHorizontalWheel(const MouseEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseButtonPress(const MouseEvent&) {
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

Point Widget::GetTopLeftCanvasPoint() const {
  Point position {};
  for (auto yoga = this->GetLayoutNode(); yoga;) {
    position.mX += YGNodeLayoutGetLeft(yoga);
    position.mY += YGNodeLayoutGetTop(yoga);

    const auto ctx = static_cast<YogaContext*>(YGNodeGetContext(yoga));
    const Widget* widget = nullptr;

    if (auto tree = std::get_if<DetachedYogaTree>(ctx)) {
      widget = tree->mLogicalParent;
    } else if (auto widgetp = std::get_if<Widget*>(ctx)) {
      widget = *widgetp;
    } else if (ctx) {
      __debugbreak();
    }
    if (widget) {
      const auto& style = widget->GetComputedStyle();
      position.mX += style.TranslateX().value_or(0);
      position.mY += style.TranslateY().value_or(0);
      yoga = YGNodeGetParent(widget->GetLayoutNode());
    } else {
      yoga = YGNodeGetParent(yoga);
    }
  }
  return position;
}

}// namespace FredEmmott::GUI::Widgets
