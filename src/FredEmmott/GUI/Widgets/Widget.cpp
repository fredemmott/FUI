// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <FredEmmott/GUI/FocusManager.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/Widgets/Focusable.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <ranges>

#include "FredEmmott/GUI/assert.hpp"
#include "FredEmmott/GUI/events/HitTestEvent.hpp"
#include "FredEmmott/GUI/events/KeyEvent.hpp"
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

void PaintOutline(
  Renderer* renderer,
  const Rect& contentRect,
  const Style& style) {
  if (!style.OutlineColor()) {
    return;
  }
  const auto thickness = style.OutlineWidth().value_or(0);
  if (thickness < std::numeric_limits<float>::epsilon()) {
    return;
  }

  const auto left = style.OutlineLeftOffset().value_or(0) + thickness / 2;
  const auto top = style.OutlineTopOffset().value_or(0) + thickness / 2;
  const auto right = style.OutlineRightOffset().value_or(0) + thickness / 2;
  const auto bottom = style.OutlineBottomOffset().value_or(0) + thickness / 2;

  const auto rect = contentRect.WithOutset(left, top, right, bottom);
  const auto radius = style.OutlineRadius().value_or(0);

  if (radius < std::numeric_limits<float>::epsilon()) {
    renderer->StrokeRect(style.OutlineColor().value(), rect, thickness);
    return;
  }

  // Like WinUI3's FocusRectManager, aim to keep the length of the straight part
  // the same, i.e. extend by the offset
  renderer->StrokeRoundedRect(
    style.OutlineColor().value(),
    rect,
    radius + std::ranges::max({left, top, bottom, right}),
    thickness);
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

Widget::Widget(
  const std::size_t id,
  const StyleClass primaryClass,
  const ImmutableStyle& immutableStyle,
  const StyleClasses& classes)
  : mOwnerWindow(Immediate::immediate_detail::tWindow),
    mPrimaryClass(primaryClass),
    mImmutableStyle(immutableStyle),
    mID(id),
    mClassList(classes),
    mYoga(YGNodeNewWithConfig(GetYogaConfig())) {
  AddStyleClass(primaryClass);
  YGNodeSetContext(mYoga.get(), new YogaContext {this});
  mStyleTransitions.reset(new StyleTransitions());
}

Widget* Widget::FromYogaNode(YGNodeConstRef node) {
  if (!node) {
    return nullptr;
  }
  const auto ctx = static_cast<YogaContext*>(YGNodeGetContext(node));
  if (!ctx) {
    return nullptr;
  }
  if (const auto it = std::get_if<Widget*>(ctx)) {
    return *it;
  }
  if (const auto it = std::get_if<DetachedYogaTree>(ctx)) {
    return it->mSelf;
  }
  return nullptr;
}

Widget* Widget::GetParent() const {
  const auto parentYoga = YGNodeGetParent(mYoga.get());
  if (!parentYoga) {
    return nullptr;
  }
  return FromYogaNode(parentYoga);
}

Widget::~Widget() {
  if (const auto fm = FocusManager::Get()) {
    fm->BeforeDestroy(this);
  }

  this->EndMouseCapture();

  const auto yogaContext
    = static_cast<YogaContext*>(YGNodeGetContext(mYoga.get()));
  YGNodeSetContext(mYoga.get(), nullptr);
  delete yogaContext;
}
void Widget::AddStyleClass(const StyleClass klass) {
  if (mClassList.contains(klass)) {
    return;
  }
  mStylesCacheKey.clear();
  mClassList.emplace(klass);
}

void Widget::ToggleStyleClass(const StyleClass klass, const bool value) {
  if (klass == mPrimaryClass && !value) {
    throw std::logic_error("Can't remove the primary class");
  }
  if (value) {
    this->AddStyleClass(klass);
    return;
  }

  if (!mClassList.contains(klass)) {
    return;
  }
  mClassList.erase(klass);
  mStylesCacheKey.clear();
}

bool Widget::IsDisabled() const {
  return ((mDirectStateFlags | mInheritedStateFlags) & StateFlags::Disabled)
    != StateFlags::Default;
}

FrameRateRequirement Widget::GetFrameRateRequirement() const noexcept {
  using enum StateFlags;
  // SmoothAnimation is as fast as we can get anyway
  if ((mDirectStateFlags & Animating) != StateFlags::Default) {
    return FrameRateRequirement::SmoothAnimation {};
  }

  return std::views::transform(
    this->mRawDirectChildren, &Widget::GetFrameRateRequirement);
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

void Widget::SetMutableStyles(const Style& styles) {
  if (styles == mMutableStyles) {
    return;
  }
  mMutableStyles = styles;
}

void Widget::AddMutableStyles(const Style& styles) {
  mMutableStyles += styles;
}

void Widget::SetDirectChildren(const std::vector<Widget*>& children) {
  std::vector<unique_ptr<Widget>> newChildren;
  for (auto child: children) {
    auto it
      = std::ranges::find(mDirectChildren, child, &unique_ptr<Widget>::get);
    if (it == mDirectChildren.end()) {
      newChildren.emplace_back(child);
    } else {
      newChildren.emplace_back(std::move(*it));
    }
  }
  mDirectChildren = std::move(newChildren);
  mRawDirectChildren = children;

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
      ctx = DetachedYogaTree {
        .mSelf = child,
        .mParent = this,
      };
    }
  }
  YGNodeSetChildren(mYoga.get(), layoutChildren.data(), layoutChildren.size());
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
  }

  PaintBackground(renderer, rect, style);
  this->PaintOwnContent(renderer, rect, style);
  this->PaintChildren(renderer);
  PaintBorder(yoga, renderer, rect, style);
  PaintOutline(renderer, rect, style);
}

void Widget::PaintChildren(Renderer* renderer) const {
  for (auto&& child: mRawDirectChildren) {
    child->Paint(renderer);
  }
}

Widget* Widget::SetChildren(const std::vector<Widget*>& children) {
  const auto foster = this->GetFosterParent();
  const auto parent = foster ? foster : this;
  if (children == parent->mRawDirectChildren) {
    return this;
  }

  parent->SetDirectChildren(children);
  return this;
}

Widget* Widget::DispatchEvent(const Event& e) {
  if (const auto it = dynamic_cast<MouseEvent const*>(&e)) {
    if (gMouseCapture) {
      auto translated = it->WithOffset(gMouseCapture->mOffset);
      return gMouseCapture->mWidget->DispatchMouseEvent(translated).mTarget;
    }

    return this->DispatchMouseEvent(*it).mTarget;
  }

  if (const auto it = dynamic_cast<KeyEvent const*>(&e)) {
    if (const auto fm = FocusManager::Get();
        const auto target = fm->GetFocusedWidget()) {
      return get<0>(*target)->DispatchKeyEvent(*it);
    }
    return nullptr;
  }

  if (const auto it = dynamic_cast<TextInputEvent const*>(&e)) {
    if (const auto fm = FocusManager::Get();
        const auto target = fm->GetFocusedWidget()) {
      return get<0>(*target)->DispatchTextInputEvent(*it);
    }
    return nullptr;
  }

  if (const auto it = dynamic_cast<const HitTestEvent*>(&e)) {
    auto relativeToSelf = *it;
    const auto yoga = this->GetLayoutNode();
    const auto display = YGNodeStyleGetDisplay(yoga);
    if (display != YGDisplayContents) {
      relativeToSelf = relativeToSelf.WithOffset({
        -YGNodeLayoutGetLeft(yoga),
        -YGNodeLayoutGetTop(yoga),
      });
    }

    for (auto&& child: mRawDirectChildren) {
      if (YGNodeStyleGetDisplay(child->GetLayoutNode()) == YGDisplayNone) {
        continue;
      }
      if (const auto target = child->DispatchEvent(relativeToSelf)) {
        return target;
      }
    }
    if (dynamic_cast<IFocusable*>(this)) {
      return this;
    }

    return nullptr;
  }

  throw std::logic_error("Unhandled event type");
}

void Widget::Tick(const std::chrono::steady_clock::time_point& now) {
  for (auto&& child: mRawDirectChildren) {
    child->Tick(now);
  }
}

void Widget::UpdateLayout() {
  for (auto&& child: mRawDirectChildren) {
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
  for (auto&& child: mRawDirectChildren) {
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
Widget* Widget::DispatchKeyEvent(const KeyEvent& e) {
  auto result = EventHandlerResult::Default;
  if (const auto it = dynamic_cast<const KeyPressEvent*>(&e)) {
    result = this->OnKeyPress(*it);
  }
  if (const auto it = dynamic_cast<const KeyReleaseEvent*>(&e)) {
    result = this->OnKeyRelease(*it);
  }

  if (result == EventHandlerResult::StopPropagation) {
    return this;
  }

  if (const auto parent = this->GetParent()) {
    return parent->DispatchKeyEvent(e);
  }

  return nullptr;
}

Widget* Widget::DispatchTextInputEvent(const TextInputEvent& e) {
  const auto result = this->OnTextInput(e);

  if (result == EventHandlerResult::StopPropagation) {
    return this;
  }

  if (const auto parent = this->GetParent()) {
    return parent->DispatchTextInputEvent(e);
  }

  return nullptr;
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

Widget::EventHandlerResult Widget::OnKeyPress(const KeyPressEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnKeyRelease(const KeyReleaseEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnTextInput(const TextInputEvent&) {
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
    const auto result = this->OnClick(event);
    if (result == EventHandlerResult::StopPropagation) {
      if (const auto fm = FocusManager::Get()) {
        fm->GivePointerFocus(this);
      }
    }
    return result;
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

bool Widget::IsChecked() const noexcept {
  using enum StateFlags;
  return ((mDirectStateFlags | mInheritedStateFlags) & Checked) == Checked;
}

void Widget::SetIsChecked(const bool value) {
  using enum StateFlags;
  if (value) {
    mDirectStateFlags |= Checked;
  } else {
    mDirectStateFlags &= ~Checked;
  }
}

Point Widget::GetTopLeftCanvasPoint() const {
  Point position {};
  for (auto widget = this; widget; widget = widget->GetParent()) {
    const auto yoga = widget->GetLayoutNode();
    position.mX += YGNodeLayoutGetLeft(yoga);
    position.mY += YGNodeLayoutGetTop(yoga);

    const auto& style = widget->GetComputedStyle();
    position.mX += style.TranslateX().value_or(0);
    position.mY += style.TranslateY().value_or(0);
  }
  return position;
}
std::string Widget::GetAccessibilityName() const {
  return std::format("{}#{}", mPrimaryClass.GetName(), this->GetID());
}

}// namespace FredEmmott::GUI::Widgets
