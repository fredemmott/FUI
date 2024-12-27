// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <core/SkRRect.h>

#include <FredEmmott/GUI/events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp>
#include <cassert>
#include <format>
#include <ranges>

#include "FredEmmott/GUI/detail/Widget/transitions.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

namespace {

template <class T>
struct yoga_default_value_t;
template <>
struct yoga_default_value_t<SkScalar> : constant_t<YGUndefined> {};
template <>
struct yoga_default_value_t<YGDisplay> : constant_t<YGDisplayFlex> {};

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
  mStyleTransitionState.reset(new StyleTransitionState());
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

void Widget::ComputeStyles(const WidgetStyles& inherited) {
  WidgetStyles merged = this->GetDefaultStyles();
  merged += inherited;
  merged += mExplicitStyles;

  constexpr auto HoverFlags
    = StateFlags::Hovered | StateFlags::HoveredInherited;
  const bool isHovered = (mStateFlags & HoverFlags) != StateFlags::Default;
  constexpr auto ActiveFlags = StateFlags::Active | StateFlags::ActiveInherited;
  const bool isActive = (mStateFlags & ActiveFlags) != StateFlags::Default;

  auto style = merged.mBase;
  if (isHovered) {
    style += merged.mHover;
  }
  if (isActive) {
    style += merged.mActive;
  }

  const auto flattenEdge = [&style]<class T>(T allEdges, T thisEdge) {
    auto& thisEdgeOpt = style.*thisEdge;
    const auto& allEdgesOpt = style.*allEdges;

    thisEdgeOpt = allEdgesOpt + thisEdgeOpt;
  };
  const auto flattenEdges
    = [&flattenEdge]<class T, std::same_as<T>... TRest>(T all, TRest... rest) {
        (flattenEdge(all, rest), ...);
      };
#define FLATTEN_EDGES(X) \
  flattenEdges( \
    &Style::m##X, \
    &Style::m##X##Left, \
    &Style::m##X##Top, \
    &Style::m##X##Right, \
    &Style::m##X##Bottom);
  FLATTEN_EDGES(Margin)
  FLATTEN_EDGES(Padding)
#undef FLATTEN_EDGES

  const auto children = this->GetDirectChildren();
  for (auto&& child: children) {
    constexpr auto clearFlags
      = StateFlags::ActiveInherited | StateFlags::HoveredInherited;
    child->mStateFlags &= ~clearFlags;
  }

  {
    using enum ComputedStyleFlags;
    if (const auto flags = this->OnComputedStyleChange(style);
        flags != Default) {
      auto stateFlags = StateFlags::Default;
      auto stateFlagValues = StateFlags::Default;

      if ((flags & InheritableActiveState) == InheritableActiveState) {
        stateFlags |= StateFlags::ActiveInherited;
        if (isActive) {
          stateFlagValues |= StateFlags::ActiveInherited;
        }
      }
      if ((flags & InheritableHoverState) == InheritableHoverState) {
        stateFlags |= StateFlags::HoveredInherited;
        if (isHovered) {
          stateFlagValues |= StateFlags::HoveredInherited;
        }
      }

      for (auto&& child: this->GetDirectChildren()) {
        child->mStateFlags &= ~stateFlags;
        child->mStateFlags |= stateFlagValues;
      }
    }
  }

  if (mComputedStyle != Style {}) {
    this->ApplyStyleTransitions(&style);
  }

  mInheritedStyles = inherited;
  mComputedStyle = style;

  const auto childStyles = merged.InheritableStyles();
  for (auto&& child: this->GetDirectChildren()) {
    child->ComputeStyles(childStyles);
  }

  const auto yoga = this->GetLayoutNode();
  const auto setYoga = [&]<class... FrontArgs>(
                         auto member, auto setter, FrontArgs&&... frontArgs) {
    const auto& optional = mComputedStyle.*member;
    if (optional.has_value()) {
      setter(yoga, std::forward<FrontArgs>(frontArgs)..., optional.value());
      return;
    }

    using T = typename std::decay_t<decltype(optional)>::value_type;
    using default_t = yoga_default_value_t<T>;
    if constexpr (requires { default_t::value; }) {
      setter(yoga, std::forward<FrontArgs>(frontArgs)..., default_t::value);
    }
  };

  setYoga(&Style::mAlignItems, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mAlignSelf, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mBottom, &YGNodeStyleSetPosition, YGEdgeBottom);
  setYoga(&Style::mDisplay, &YGNodeStyleSetDisplay);
  setYoga(&Style::mFlexDirection, &YGNodeStyleSetFlexDirection);
  setYoga(&Style::mGap, &YGNodeStyleSetGap, YGGutterAll);
  setYoga(&Style::mHeight, &YGNodeStyleSetHeight);
  setYoga(&Style::mLeft, &YGNodeStyleSetPosition, YGEdgeLeft);
  setYoga(&Style::mMarginBottom, &YGNodeStyleSetMargin, YGEdgeBottom);
  setYoga(&Style::mMarginLeft, &YGNodeStyleSetMargin, YGEdgeLeft);
  setYoga(&Style::mMarginRight, &YGNodeStyleSetMargin, YGEdgeRight);
  setYoga(&Style::mMarginTop, &YGNodeStyleSetMargin, YGEdgeTop);
  setYoga(&Style::mPaddingBottom, &YGNodeStyleSetPadding, YGEdgeBottom);
  setYoga(&Style::mPaddingLeft, &YGNodeStyleSetPadding, YGEdgeLeft);
  setYoga(&Style::mPaddingRight, &YGNodeStyleSetPadding, YGEdgeRight);
  setYoga(&Style::mPaddingTop, &YGNodeStyleSetPadding, YGEdgeTop);
  setYoga(&Style::mRight, &YGNodeStyleSetPosition, YGEdgeRight);
  setYoga(&Style::mTop, &YGNodeStyleSetPosition, YGEdgeTop);
  setYoga(&Style::mWidth, &YGNodeStyleSetWidth);
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
  x -= YGNodeLayoutGetLeft(layout);
  y -= YGNodeLayoutGetTop(layout);
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
    mStateFlags &= ~StateFlags::Hovered;

    static constexpr auto invalid = -std::numeric_limits<SkScalar>::infinity();
    translated->mPoint = {-invalid, -invalid};
  } else {
    mStateFlags |= StateFlags::Hovered;
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
      if ((mStateFlags & StateFlags::Hovered) == StateFlags::Hovered) {
        mStateFlags |= flags;
      } else {
        mStateFlags &= ~flags;
      }
      break;
    }
    case EventKind::ButtonRelease: {
      constexpr auto flags = StateFlags::Hovered | StateFlags::MouseDownTarget;
      if ((mStateFlags & flags) == flags) {
        isClick = true;
      }
      mStateFlags &= ~(StateFlags::MouseDownTarget | StateFlags::Active);
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

  if (isClick) {
    result = this->OnClick(translated.get());
  }

  return result;
}

Widget::ComputedStyleFlags Widget::OnComputedStyleChange(const Style&) {
  auto ret = ComputedStyleFlags::Default;
  if ((mStateFlags & StateFlags::HoveredInherited) != StateFlags::Default) {
    ret |= ComputedStyleFlags::InheritableHoverState;
  }
  if ((mStateFlags & StateFlags::ActiveInherited) != StateFlags::Default) {
    ret |= ComputedStyleFlags::InheritableActiveState;
  }
  return ret;
}

}// namespace FredEmmott::GUI::Widgets