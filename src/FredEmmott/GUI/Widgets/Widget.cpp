// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <core/SkRRect.h>

#include <FredEmmott/GUI/Events/MouseButtonPressEvent.hpp>
#include <FredEmmott/GUI/Events/MouseButtonReleaseEvent.hpp>
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
  });
  return sInstance.get();
}

void PaintBackground(SkCanvas* canvas, const SkRect& rect, const Style& style) {
  if (!style.mBackgroundColor) {
    return;
  }

  SkPaint paint;
  paint.setColor(style.mBackgroundColor.value());

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
  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setColor(style.mBorderColor.value());
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
}

bool Widget::IsHovered() const noexcept {
  return (mStateFlags & StateFlags::Hovered) == StateFlags::Hovered;
}

Widget::~Widget() = default;

void Widget::ComputeStyles(const WidgetStyles& inherited) {
  WidgetStyles merged = this->GetDefaultStyles();
  merged += inherited;
  merged += mExplicitStyles;

  auto style = merged.mBase;
  if (this->IsHovered()) {
    style += merged.mHover;
  }

  if (mComputedStyle != style) {
    this->OnComputedStyleChange(style);
  }
  mInheritedStyles = inherited;
  mComputedStyle = style;

  const auto layout = this->GetLayoutNode();
  const auto setYoga
    = [&]<class... Front>(auto member, auto setter, Front&&... args) {
        const auto& value = mComputedStyle.*member;
        if (value) {
          setter(layout, std::forward<Front>(args)..., *value);
        }
      };
  setYoga(&Style::mAlignItems, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mAlignSelf, &YGNodeStyleSetAlignSelf);
  setYoga(&Style::mFlexDirection, &YGNodeStyleSetFlexDirection);
  setYoga(&Style::mGap, &YGNodeStyleSetGap, YGGutterAll);
  setYoga(&Style::mHeight, &YGNodeStyleSetHeight);
  setYoga(&Style::mMargin, &YGNodeStyleSetMargin, YGEdgeAll);
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

  this->PaintOwnContent(canvas, style);

  const auto children = this->GetChildren();
  if (children.empty()) {
    return;
  }

  const auto layout = this->GetLayoutNode();
  const auto x = YGNodeLayoutGetLeft(layout);
  const auto y = YGNodeLayoutGetTop(layout);

  canvas->save();
  canvas->translate(x, y);
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

  if (const auto it = dynamic_cast<const MouseMoveEvent*>(e)) {
    Translate(it);
  } else if (const auto it = dynamic_cast<const MouseButtonPressEvent*>(e)) {
    Translate(it);
  } else if (const auto it = dynamic_cast<const MouseButtonReleaseEvent*>(e)) {
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

  if (const auto it = dynamic_cast<const MouseButtonPressEvent*>(e)) {
    if (IsHovered()) {
      mStateFlags |= StateFlags::MouseDownTarget;
    } else {
      mStateFlags &= ~StateFlags::MouseDownTarget;
    }
  } else if (const auto it = dynamic_cast<const MouseButtonReleaseEvent*>(e)) {
    constexpr auto clickFlags
      = StateFlags::Hovered | StateFlags::MouseDownTarget;
    if ((mStateFlags & clickFlags) == clickFlags) {
      isClick = true;
    }
    mStateFlags &= ~StateFlags::MouseDownTarget;
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

}// namespace FredEmmott::GUI::Widgets