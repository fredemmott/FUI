// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <core/SkRRect.h>

#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/immediate_detail.hpp>
#include <FredEmmott/GUI/events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp>
#include <cassert>
#include <format>
#include <ranges>

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

namespace {

YGConfigRef GetYogaConfig() {
  static unique_ptr<YGConfig> sInstance;
  static std::once_flag sOnceFlag;
  std::call_once(sOnceFlag, [&ret = sInstance]() {
    ret.reset(YGConfigNew());
    YGConfigSetUseWebDefaults(ret.get(), true);
    YGConfigSetPointScaleFactor(ret.get(), 0.0f);
  });
  return sInstance.get();
}

void PaintBackground(SkCanvas* canvas, const SkRect& rect, const Style& style) {
  if (!style.mBackgroundColor) {
    return;
  }

  auto paint = style.mBackgroundColor->GetPaint(rect);

  if (!style.mBorderRadius) {
    canvas->drawRect(rect, paint);
    return;
  }

  const auto radius = style.mBorderRadius.value();

  paint.setAntiAlias(true);
  canvas->drawRoundRect(rect, radius, radius, paint);
}

void PaintBorder(SkCanvas* canvas, const SkRect& rect, const Style& style) {
  if (!(style.mBorderColor && style.mBorderWidth)) {
    return;
  }

  // TODO: YGNodeStyleSetBorder
  const auto bw = style.mBorderWidth.value();
  auto paint = style.mBorderColor->GetPaint(rect);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(bw);

  SkRect border = rect;
  border.inset(bw / 2, bw / 2);

  if (!style.mBorderRadius) {
    canvas->drawRect(border, paint);
    return;
  }

  const auto radius = style.mBorderRadius.value();

  paint.setAntiAlias(true);
  canvas->drawRoundRect(border, radius, radius, paint);
}
}// namespace

Widget::Widget(std::size_t id)
  : mID(id), mYoga(YGNodeNewWithConfig(GetYogaConfig())) {
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

Widget::~Widget() = default;

bool Widget::IsDisabled() const {
  return ((mDirectStateFlags | mInheritedStateFlags) & StateFlags::Disabled)
    != StateFlags::Default;
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

void Widget::SetExplicitStyles(const WidgetStyles& styles) {
  if (styles == mExplicitStyles) {
    return;
  }
  mExplicitStyles = styles;

  this->ComputeStyles(mInheritedStyles);
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
  const auto yoga = this->GetLayoutNode();
  const auto rect = SkRect::MakeXYWH(
    YGNodeLayoutGetLeft(yoga),
    YGNodeLayoutGetTop(yoga),
    YGNodeLayoutGetWidth(yoga),
    YGNodeLayoutGetHeight(yoga));

  PaintBackground(canvas, rect, style);
  PaintBorder(canvas, rect, style);

  this->PaintOwnContent(canvas, rect, style);

  const auto children = this->GetDirectChildren();
  if (children.empty()) {
    return;
  }

  canvas->save();
  canvas->translate(rect.x(), rect.y());
  for (auto&& child: children) {
    child->Paint(canvas);
  }
  canvas->restore();
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
    (void)this->DispatchMouseEvent(it);
    return;
  }
  // whut?
  __debugbreak();
}

Widget::EventHandlerResult Widget::DispatchMouseEvent(const MouseEvent* e) {
  auto point = e->mPoint;
  auto& [x, y] = point;

  const auto layout = this->GetLayoutNode();
  const auto display = YGNodeStyleGetDisplay(layout);
  if (display != YGDisplayContents) {
    x -= YGNodeLayoutGetLeft(layout);
    y -= YGNodeLayoutGetTop(layout);
  }
  const auto w = YGNodeLayoutGetWidth(layout);
  const auto h = YGNodeLayoutGetHeight(layout);

  unique_ptr<MouseEvent> translated;
  const auto Translate
    = [&translated, point]<std::derived_from<MouseEvent> T>(const T* event) {
        translated.reset(new T(*event));
        translated->mPoint = point;
        return static_cast<const T&>(*translated);
      };

  enum class EventKind {
    Unknown,
    Move,
    ButtonPress,
    ButtonRelease,
  };
  auto kind = EventKind::Unknown;

  if (const auto it = dynamic_cast<const MouseMoveEvent*>(e)) {
    kind = EventKind::Move;
    Translate(it);
  } else if (const auto it = dynamic_cast<const MouseButtonPressEvent*>(e)) {
    kind = EventKind::ButtonPress;
    Translate(it);
  } else if (const auto it = dynamic_cast<const MouseButtonReleaseEvent*>(e)) {
    kind = EventKind::ButtonRelease;
    Translate(it);
  }

  if (!translated) {
#ifndef NDEBUG
    __debugbreak();
#endif
    return EventHandlerResult::Default;
  }

  if (x < 0 || y < 0 || x > w || y > h) {
    mDirectStateFlags &= ~StateFlags::Hovered;

    if (display != YGDisplayContents) {
      static constexpr auto invalid
        = -std::numeric_limits<SkScalar>::infinity();
      translated->mPoint = {-invalid, -invalid};
    }
  } else {
    mDirectStateFlags |= StateFlags::Hovered;
  }

  bool isClick = false;

  switch (kind) {
    case EventKind::Unknown:
#ifndef NDEBUG
      __debugbreak();
#endif
      break;
    case EventKind::Move:
      break;
    case EventKind::ButtonPress: {
      constexpr auto flags = StateFlags::MouseDownTarget | StateFlags::Active;
      if ((mDirectStateFlags & StateFlags::Hovered) == StateFlags::Hovered) {
        mDirectStateFlags |= flags;
      } else {
        mDirectStateFlags &= ~flags;
      }
      break;
    }
    case EventKind::ButtonRelease: {
      constexpr auto flags = StateFlags::Hovered | StateFlags::MouseDownTarget;
      if ((mDirectStateFlags & flags) == flags) {
        isClick = true;
      }
      mDirectStateFlags &= ~(StateFlags::MouseDownTarget | StateFlags::Active);
      break;
    }
  }

  auto result = EventHandlerResult::Default;
  // Always propagate unconditionally to allow correct internal states
  for (auto&& child: this->GetDirectChildren()) {
    if (
      child->DispatchMouseEvent(translated.get())
      == EventHandlerResult::StopPropagation) {
      result = EventHandlerResult::StopPropagation;
    }
  }

  if (result == EventHandlerResult::StopPropagation) {
    return result;
  }

  if (isClick && !this->IsDisabled()) {
    result = this->OnClick(translated.get());
  }

  return result;
}

}// namespace FredEmmott::GUI::Widgets