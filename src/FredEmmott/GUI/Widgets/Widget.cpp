// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <core/SkRRect.h>

#include <FredEmmott/GUI/events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/events/MouseButtonReleaseEvent.hpp>
#include <format>
#include <ranges>

#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Widgets {

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

template <class T>
struct TransitionData {
  using option_type = TransitionData;
};

template <animatable T>
struct TransitionData<T> {
  using option_type = std::optional<TransitionData>;
  using time_point = std::chrono::steady_clock::time_point;
  T mStartValue;
  time_point mStartTime;
  T mEndValue;
  time_point mEndTime;

  [[nodiscard]] T Evaluate(const time_point& now) const noexcept {
    if (now < mStartTime) {
      return mStartValue;
    }
    if (now > mEndTime) {
      return mEndValue;
    }
    const auto duration = mEndTime - mStartTime;
    const auto elapsed = now - mStartTime;
    const auto r = static_cast<double>(elapsed.count()) / duration.count();
    return static_cast<T>(mStartValue + ((mEndValue - mStartValue) * r));
  }
};

struct Widget::StyleTransitionState {
#define DECLARE_TRANSITION_DATA(X) \
  FUI_NO_UNIQUE_ADDRESS \
  TransitionData<decltype(Style::m##X)::value_type>::option_type m##X;
  FUI_STYLE_PROPERTIES(DECLARE_TRANSITION_DATA)
#undef TRANSITION_DATA
};

Widget::Widget(std::size_t id)
  : mID(id), mYoga(YGNodeNewWithConfig(GetYogaConfig())) {
  YGNodeSetContext(mYoga.get(), this);
  mStyleTransitionState.reset(new StyleTransitionState());
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

  if (mComputedStyle != Style {}) {
    this->ApplyStyleTransitions(&style);
  }

  if (mComputedStyle != style) {
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

      for (auto&& child: mChildren) {
        child->mStateFlags &= ~stateFlags;
        child->mStateFlags |= stateFlagValues;
      }
    }
  }
  mInheritedStyles = inherited;
  mComputedStyle = style;

  const auto yoga = this->GetLayoutNode();
  const auto setYoga = [&]<class... Front>(
                         auto member, auto setter, Front&&... args) {
    const auto& value = mComputedStyle.*member;
    using T = std::decay_t<decltype(value)>::value_type;
    if constexpr (std::same_as<T, SkScalar>) {
      setter(yoga, std::forward<Front>(args)..., value.value_or(YGUndefined));
    } else if (value) {
      setter(yoga, std::forward<Front>(args)..., *value);
    }
  };
  setYoga(&Style::mAlignItems, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mAlignSelf, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mFlexDirection, &YGNodeStyleSetFlexDirection);
  setYoga(&Style::mGap, &YGNodeStyleSetGap, YGGutterAll);
  setYoga(&Style::mHeight, &YGNodeStyleSetHeight);
  setYoga(&Style::mMargin, &YGNodeStyleSetMargin, YGEdgeAll);
  setYoga(&Style::mMarginBottom, &YGNodeStyleSetMargin, YGEdgeBottom);
  setYoga(&Style::mMarginLeft, &YGNodeStyleSetMargin, YGEdgeLeft);
  setYoga(&Style::mMarginRight, &YGNodeStyleSetMargin, YGEdgeRight);
  setYoga(&Style::mMarginTop, &YGNodeStyleSetMargin, YGEdgeTop);
  setYoga(&Style::mPadding, &YGNodeStyleSetPadding, YGEdgeAll);
  setYoga(&Style::mPaddingBottom, &YGNodeStyleSetPadding, YGEdgeBottom);
  setYoga(&Style::mPaddingLeft, &YGNodeStyleSetPadding, YGEdgeLeft);
  setYoga(&Style::mPaddingRight, &YGNodeStyleSetPadding, YGEdgeRight);
  setYoga(&Style::mPaddingTop, &YGNodeStyleSetPadding, YGEdgeTop);
  setYoga(&Style::mWidth, &YGNodeStyleSetWidth);

  const auto childStyles = merged.InheritableStyles();
  for (auto&& child: this->GetChildren()) {
    child->ComputeStyles(childStyles);
  }
}

void Widget::SetExplicitStyles(const WidgetStyles& styles) {
  if (styles == mExplicitStyles) {
    return;
  }
  mExplicitStyles = styles;

  this->ComputeStyles(mInheritedStyles);
}

std::span<Widget* const> Widget::GetChildren() const noexcept {
  return mStorageForGetChildren;
}

void Widget::SetChildren(const std::vector<Widget*>& children) {
  if (children == mStorageForGetChildren) {
    return;
  }

  const auto layout = this->GetLayoutNode();
  YGNodeRemoveAllChildren(layout);

  std::vector<unique_ptr<Widget>> newChildren;
  for (auto child: children) {
    auto it = std::ranges::find(mChildren, child, &unique_ptr<Widget>::get);
    if (it == mChildren.end()) {
      newChildren.emplace_back(child);
    } else {
      newChildren.emplace_back(std::move(*it));
    }
  }
  mChildren = std::move(newChildren);
  mStorageForGetChildren = children;

  const auto childLayouts
    = std::views::transform(mChildren, &Widget::GetLayoutNode)
    | std::ranges::to<std::vector>();
  YGNodeSetChildren(layout, childLayouts.data(), childLayouts.size());
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

  const auto children = this->GetChildren();
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
  for (auto&& child: this->GetChildren()) {
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
void Widget::ApplyStyleTransitions(Style* newStyle) {
  auto state = mStyleTransitionState.get();
  const auto now = std::chrono::steady_clock::now();

  const auto apply = [now, newStyle, oldStyle = &mComputedStyle, state](
                       auto styleP, auto stateP) {
    auto oldOpt = oldStyle->*styleP;
    auto& newOpt = newStyle->*styleP;
    if (oldOpt == newOpt) {
      return;
    }
    using TValueOption = std::decay_t<decltype(newOpt)>;
    using TValue = TValueOption::value_type;
    const auto oldValue = oldOpt.value_or(TValue {});
    const auto newValue = newOpt.value_or(TValue {});
    if (std::isnan(oldValue) || std::isnan(newValue)) {
      return;
    }

    if (std::isnan(newValue)) {
      __debugbreak();
    }

    auto& transitionState = state->*stateP;
    if (transitionState.has_value()) {
      if (transitionState->mEndTime < now) {
        transitionState.reset();
        return;
      }
      if (transitionState->mEndValue != newValue) {
        transitionState->mStartValue = transitionState->Evaluate(now);
        transitionState->mStartTime = now;
        transitionState->mEndTime = now + newOpt.transition().mDuration;
        transitionState->mEndValue = newValue;
        newOpt = transitionState->mStartValue;
        return;
      }
      newOpt = transitionState->Evaluate(now);
      if (std::isnan(*newOpt)) {
        __debugbreak();
      }
      return;
    }
    transitionState = {
      .mStartValue = oldValue,
      .mStartTime = now,
      .mEndValue = newValue,
      .mEndTime = now + newOpt.transition().mDuration,
    };
    newOpt = oldValue;
    if (std::isnan(*newOpt)) {
      __debugbreak();
    }
  };

  const auto applyIfHasTransition
    = [newStyle, apply](auto styleP, auto stateP) {
        auto& prop = newStyle->*styleP;
        if constexpr (requires { prop.has_transition(); }) {
          if (prop.has_transition()) {
            apply(styleP, stateP);
          }
        }
      };

#define APPLY_TRANSITION(X) \
  { \
    const auto propName = #X; \
    applyIfHasTransition(&Style::m##X, &StyleTransitionState::m##X); \
  }
  FUI_STYLE_PROPERTIES(APPLY_TRANSITION)
#undef APPLY_TRANSITION
}

}// namespace FredEmmott::GUI::Widgets