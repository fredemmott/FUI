// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <FredEmmott/GUI/FocusManager.hpp>
#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/Widgets/Focusable.hpp>
#include <FredEmmott/GUI/Window.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/events/HitTestEvent.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <FredEmmott/utility/almost_equal.hpp>
#include <felly/overload.hpp>
#include <ranges>

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

  const auto brush = *style.BackgroundColor();

  const CornerRadius radii {
    style.BorderTopLeftRadius().value_or(0),
    style.BorderTopRightRadius().value_or(0),
    style.BorderBottomRightRadius().value_or(0),
    style.BorderBottomLeftRadius().value_or(0),
  };

  if (radii.IsEmpty()) {
    renderer->FillRect(brush, rect);
    return;
  }

  renderer->FillRoundedRect(brush, rect, radii);
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
  const CornerRadius radii {
    style.OutlineTopLeftRadius().value_or(0),
    style.OutlineTopRightRadius().value_or(0),
    style.OutlineBottomRightRadius().value_or(0),
    style.OutlineBottomLeftRadius().value_or(0),
  };

  if (radii.IsEmpty()) {
    renderer->StrokeRect(style.OutlineColor().value(), rect, thickness);
    return;
  }

  // Like WinUI3's FocusRectManager, aim to keep the length of the straight part
  // the same, i.e. extend by the offset
  const auto pad = std::ranges::max({left, top, bottom, right});
  renderer->StrokeRoundedRect(
    style.OutlineColor().value(),
    rect,
    CornerRadius {
      radii.GetTopLeft() + pad,
      radii.GetTopRight() + pad,
      radii.GetBottomRight() + pad,
      radii.GetBottomLeft() + pad,
    },
    EdgeFlags::All,
    thickness);
}

void PaintBorder(
  const YGNode* yoga,
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

  static constexpr auto Eps = 0.1f;
  if (top < Eps && left < Eps && right < Eps && bottom < Eps) {
    return;
  }

  const auto allSame = (top == left) && (top == right) && (top == bottom);
  const auto borderRect
    = contentRect.WithInset(left / 2, top / 2, right / 2, bottom / 2);

  const auto brush = *style.BorderColor();

  const CornerRadius radii {
    style.BorderTopLeftRadius().value_or(0),
    style.BorderTopRightRadius().value_or(0),
    style.BorderBottomRightRadius().value_or(0),
    style.BorderBottomLeftRadius().value_or(0),
  };
  if (!radii.IsEmpty()) {
    auto edges = EdgeFlags::All;
    if (!allSame) {
      edges = EdgeFlags::None;

      struct NoOpValidator final {
        void Check(const float) {};
      };
      struct DebugValidator final {
        void Check(const float value) {
          if (mValue < Eps) {
            mValue = value;
          } else {
            FUI_ALWAYS_ASSERT(
              utility::almost_equal(mValue, value),
              "With border radius set, all borders must be the same or zero");
          }
        }

       private:
        float mValue {};
      };
      std::conditional_t<Config::Debug, DebugValidator, NoOpValidator>
        validator;

      if (left >= Eps) {
        validator.Check(left);
        edges |= EdgeFlags::Left;
      }
      if (top >= Eps) {
        validator.Check(top);
        edges |= EdgeFlags::Top;
      }
      if (right >= Eps) {
        validator.Check(right);
        edges |= EdgeFlags::Right;
      }
      if (bottom >= Eps) {
        validator.Check(bottom);
        edges |= EdgeFlags::Bottom;
      }
    }
    renderer->StrokeRoundedRect(brush, borderRect, radii, edges, top);
    return;
  }

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
      brush, contentRect.GetBottomRight(), contentRect.GetBottomLeft(), bottom);
  }
  if (left > Eps) {
    renderer->DrawLine(
      brush, contentRect.GetBottomLeft(), contentRect.GetTopLeft(), left);
  }
}
}// namespace

Widget::Widget(
  Window* const ownerWindow,
  const StyleClass primaryClass,
  const ImmutableStyle& immutableStyle,
  const StyleClasses& classes)
  : mOwnerWindow(ownerWindow),
    mPrimaryClass(primaryClass),
    mImmutableStyle(immutableStyle),
    mClassList(classes),
    mYoga(YGNodeNewWithConfig(GetYogaConfig())) {
  AddStyleClass(primaryClass);
  YGNodeSetContext(mYoga.get(), this);
  mStyleTransitions.reset(new StyleTransitions());
}

Widget* Widget::FromYogaNode(const YGNode* const node) {
  if (!node) {
    return nullptr;
  }
  // While we have yoga nodes that aren't widgets, they do not have
  // a yoga context. `nullptr` is `nullptr`, whether it's a `void*` or a
  // `Widget*`, so this is correct either way.
  //
  // If we end up needing to store context for non-Widgets, we should decide on
  // a shared storage model (e.g. variant), 'magic' header bytes, or similar
  // safety measure, but for now, we just have widgets-or-nullptr.
  return static_cast<Widget*>(YGNodeGetContext(node));
}

Widget::~Widget() {
  mDestructionInProgress = true;
  this->EndMouseCapture();

  this->SetStructuralChildren({}, nullptr);

  this->GetOwnerWindow()->GetFocusManager()->BeforeDestroy(this);

  YGNodeSetContext(mYoga.get(), nullptr);
}

void Widget::SetImmediateContext(
  Widget* const logicalParent,
  const id_type id) {
  FUI_ASSERT(!mID);
  FUI_ASSERT(!mLogicalParent);
  FUI_ASSERT(!mStructuralParent);
  mLogicalParent = logicalParent;
  mStructuralParent = logicalParent->GetStructuralParentForLogicalChildren();
  mID = id;
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
    this->mRawStructuralChildren, &Widget::GetFrameRateRequirement);
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

bool Widget::IsHovered() const {
  return ((mDirectStateFlags | mInheritedStateFlags) & StateFlags::Hovered)
    == StateFlags::Hovered;
}

bool Widget::IsActive() const {
  return ((mDirectStateFlags | mInheritedStateFlags) & StateFlags::Active)
    == StateFlags::Active;
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

void Widget::SetStructuralChildren(
  const std::vector<Widget*>& children,
  Widget* const logicalParent) {
  if (children == mRawStructuralChildren) {
    return;
  }

  if (children.empty()) {
    mStructuralChildren.clear();
    mRawStructuralChildren.clear();
    YGNodeSetChildren(mYoga.get(), nullptr, 0);
    return;
  }

  FUI_ASSERT(logicalParent);
  std::vector<std::unique_ptr<Widget>> ownedChildren;
  ownedChildren.reserve(children.size());
  for (auto child: children) {
    auto it = std::ranges::find(
      mStructuralChildren, child, &std::unique_ptr<Widget>::get);
    if (it != mStructuralChildren.end()) {
      ownedChildren.emplace_back(std::move(*it));
      continue;
    }

    FUI_ASSERT(
      child->mStructuralParent == nullptr || child->mStructuralParent == this);
    FUI_ASSERT(
      child->mLogicalParent == nullptr
      || child->mLogicalParent == logicalParent);
    child->mStructuralParent = this;
    child->mLogicalParent = logicalParent;
    ownedChildren.emplace_back(child);
  }
  mStructuralChildren = std::move(ownedChildren);
  mRawStructuralChildren = children;

  std::vector<YGNode*> layoutChildren;
  layoutChildren.reserve(children.end() - children.begin());
  for (auto&& child: children) {
    if (!child->mClassList.contains(PseudoClasses::LayoutOrphan)) {
      layoutChildren.push_back(child->GetLayoutNode());
      continue;
    }
  }
  YGNodeSetChildren(mYoga.get(), layoutChildren.data(), layoutChildren.size());
}

void Widget::Paint(Renderer* renderer) const {
  const auto& style = mComputedStyle;

  if (style.Display() == Display::None) {
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
  const Rect rect {Size {
    YGNodeLayoutGetWidth(yoga),
    YGNodeLayoutGetHeight(yoga),
  }};
  if (
    style.HasRotate() && !utility::almost_equal(style.Rotate().value(), 0.f)) {
    renderer->Rotate(
      style.Rotate().value(),
      Point {
        style.TransformOriginX().value_or(0.f) * rect.GetWidth(),
        style.TransformOriginY().value_or(0.f) * rect.GetHeight(),
      });
  }

  const auto scaleX = style.ScaleX().value_or(1.f);
  const auto scaleY = style.ScaleY().value_or(1.f);
  if (
    std::min(scaleX, scaleY) + std::numeric_limits<float>::epsilon() < 1.0f
    || std::max(scaleX, scaleY)
      > 1.0f + std::numeric_limits<float>::epsilon()) {
    const auto oldWidth = rect.GetWidth();
    const auto oldHeight = rect.GetHeight();

    const auto dx = style.TransformOriginX().value_or(0.f) * oldWidth;
    const auto dy = style.TransformOriginY().value_or(0.f) * oldHeight;

    renderer->Translate(dx, dy);
    renderer->Scale(scaleX, scaleY);
    renderer->Translate(-dx, -dy);
  }

  PaintBackground(renderer, rect, style);
  this->PaintOwnContent(renderer, rect, style);
  this->PaintChildren(renderer);
  PaintBorder(yoga, renderer, rect, style);
  PaintOutline(renderer, rect, style);
}

void Widget::PaintChildren(Renderer* renderer) const {
  for (auto&& child: mRawStructuralChildren) {
    child->Paint(renderer);
  }
}

void Widget::OnMouseEnter(const MouseEvent&) {}

void Widget::OnMouseLeave(const MouseEvent&) {}

Widget* Widget::DispatchEvent(const Event& e) {
  if (const auto it = dynamic_cast<MouseEvent const*>(&e)) {
    if (gMouseCapture) {
      const auto translated = it->WithOffset(gMouseCapture->mOffset);
      return gMouseCapture->mWidget->DispatchMouseEvent(translated).mTarget;
    }

    return this->DispatchMouseEvent(*it).mTarget;
  }

  const auto fm = this->GetOwnerWindow()->GetFocusManager();
  FUI_ASSERT(fm);
  if (const auto it = dynamic_cast<KeyEvent const*>(&e)) {
    if (const auto target = fm->GetFocusedWidget()) {
      const auto widget = get<0>(*target);
      return widget->DispatchKeyEvent(*it);
    }
    return nullptr;
  }

  if (const auto it = dynamic_cast<TextInputEvent const*>(&e)) {
    if (const auto target = fm->GetFocusedWidget()) {
      const auto widget = get<0>(*target);
      return widget->DispatchTextInputEvent(*it);
    }
    return nullptr;
  }

  if (const auto it = dynamic_cast<const HitTestEvent*>(&e)) {
    auto relativeToSelf = *it;
    const auto yoga = this->GetLayoutNode();
    const auto display = YGNodeStyleGetDisplay(yoga);
    if (display == YGDisplayNone) {
      return nullptr;
    }

    if (display != YGDisplayContents) {
      relativeToSelf = relativeToSelf.WithOffset({
        -YGNodeLayoutGetLeft(yoga),
        -YGNodeLayoutGetTop(yoga),
      });
    }

    for (auto&& child: mRawStructuralChildren) {
      if (const auto target = child->DispatchEvent(relativeToSelf)) {
        return target;
      }
    }

    if (
      relativeToSelf.mPoint.mX < 0 || relativeToSelf.mPoint.mY < 0
      || relativeToSelf.mPoint.mX >= YGNodeLayoutGetWidth(yoga)
      || relativeToSelf.mPoint.mY >= YGNodeLayoutGetHeight(yoga)) {
      return nullptr;
    }

    if (dynamic_cast<IFocusable*>(this)) {
      return this;
    }

    return nullptr;
  }

  throw std::logic_error("Unhandled event type");
}

void Widget::Tick(const std::chrono::steady_clock::time_point& now) {
  for (auto&& child: mRawStructuralChildren) {
    child->Tick(now);
  }
}

Widget::MouseEventResult Widget::DispatchMouseEvent(
  const MouseEvent& parentEvent) {
  if (GetComputedStyle().PointerEvents() == PointerEvents::None) {
    return {};
  }
  if (IsDisabled()) {
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
  const bool wasHovered
    = (mDirectStateFlags & StateFlags::Hovered) == StateFlags::Hovered;
  // `x >= w` because `x + w` is an *exclusive* bound for our widget; the next
  // widget may start at exactly x + w
  //
  // You can test this by creating two touching square widgets with hover
  // effects (e.g. background-color), and seeing if it's possible to activate
  // both hover effects at the same time.
  //
  // For example, in Win32 windows with a modern title bar, the min/max/close
  // buttons are all touching, and with `x > w`, it's possible to activate
  // two buttons at once
  if (
    (x < 0 || y < 0 || x >= w || y >= h)
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
  const bool isHovered
    = (mDirectStateFlags & StateFlags::Hovered) == StateFlags::Hovered;
  if (
    holds_alternative<MouseEvent::ButtonPressEvent>(event.mDetail)
    && isHovered) {
    mDirectStateFlags |= StateFlags::Active;
  }
  if (holds_alternative<MouseEvent::ButtonReleaseEvent>(event.mDetail)) {
    mDirectStateFlags &= ~StateFlags::Active;
  }

  if (event.IsValid()) {
    // The same offset will be applied for captured mouse inputs, so we want
    // to use the same offset we were called with, not the post-processed one,
    // as we'll do the same processing when we receive the captured event.
    mMouseCaptureOffset = parentEvent.mOffset;
  }

  if (isHovered && !wasHovered) {
    this->OnMouseEnter(event);
  } else if (wasHovered && !isHovered) {
    this->OnMouseLeave(event);
  }

  // Always propagate unconditionally to allow correct internal states
  for (auto&& child: mRawStructuralChildren) {
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

  const bool aimedAtThis
    = event.IsValid() || (gMouseCapture && gMouseCapture->mWidget == this);
  if (!aimedAtThis) {
    return result;
  }

  result.mResult = std::visit(
    felly::overload {
      [&](const MouseEvent::ButtonPressEvent&) {
        return this->OnMouseButtonPress(event);
      },
      [&](const MouseEvent::ButtonReleaseEvent&) {
        mDirectStateFlags &= ~StateFlags::Active;
        return this->OnMouseButtonRelease(event);
      },
      [&](const MouseEvent::MoveEvent&) { return this->OnMouseMove(event); },
      [&, this](const MouseEvent::HoverEvent&) {
        mWasStationaryHovered = event;
        return this->OnMouseHover(event);
      },
      [&](const MouseEvent::HorizontalWheelEvent&) {
        return this->OnMouseHorizontalWheel(event);
      },
      [&](const MouseEvent::VerticalWheelEvent&) {
        return this->OnMouseVerticalWheel(event);
      },
    },
    event.mDetail);

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
    this->GetOwnerWindow()->GetFocusManager()->GiveVisibleFocus(this);
    return this;
  }

  if (const auto parent = this->GetStructuralParentOrNull()) {
    return parent->DispatchKeyEvent(e);
  }

  return nullptr;
}

Widget* Widget::DispatchTextInputEvent(const TextInputEvent& e) {
  const auto result = this->OnTextInput(e);

  if (result == EventHandlerResult::StopPropagation) {
    return this;
  }

  if (const auto parent = this->GetStructuralParentOrNull()) {
    return parent->DispatchTextInputEvent(e);
  }

  return nullptr;
}

Widget::EventHandlerResult Widget::OnMouseMove(const MouseEvent&) {
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseHover(const MouseEvent&) {
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
  return EventHandlerResult::Default;
}

Widget::EventHandlerResult Widget::OnMouseButtonRelease(
  const MouseEvent& event) {
  const auto result = this->OnClick(event);
  if (result == EventHandlerResult::StopPropagation) {
    this->GetOwnerWindow()->GetFocusManager()->GiveImplicitFocus(this);
  }
  return result;
}

void Widget::StartMouseCapture() {
  FUI_ASSERT(!gMouseCapture);
  gMouseCapture = MouseCapture {this, mMouseCaptureOffset};
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

Point Widget::GetTopLeftCanvasPoint(const Widget* const relativeTo) const {
  Point position {};
  for (auto widget = this; widget && widget != relativeTo;
       widget = widget->GetStructuralParentOrNull()) {
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
