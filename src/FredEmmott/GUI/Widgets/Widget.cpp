// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "Widget.hpp"

#include <core/SkRRect.h>

#include <format>

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

  auto style = merged.mDefault;
  if (this->IsHovered()) {
    style += merged.mHover;
  }

  if (const auto instance = this->GetInstanceStyles(style)) {
    style += *instance;
    style += mExplicitStyles.mDefault;
    if (this->IsHovered()) {
      style += mExplicitStyles.mHover;
    }
  }

  mInheritedStyles = inherited;
  mComputedStyle = style;

  const auto layout = this->GetLayoutNode();
  const auto setYoga = [&](auto member, auto setter) {
    const auto& value = mComputedStyle.*member;
    if (value) {
      setter(layout, *value);
    }
  };
  setYoga(&Style::mWidth, &YGNodeStyleSetWidth);
  setYoga(&Style::mHeight, &YGNodeStyleSetHeight);

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
    this->DispatchMouseEvent(it);
    return;
  }
  // whut?
  __debugbreak();
}

void Widget::DispatchMouseEvent(const MouseEvent* e) {
  auto point = e->mPoint;
  auto& [x, y] = point;
  const auto layout = this->GetLayoutNode();
  x -= YGNodeLayoutGetLeft(layout);
  y -= YGNodeLayoutGetTop(layout);
  const auto w = YGNodeLayoutGetWidth(layout);
  const auto h = YGNodeLayoutGetHeight(layout);

  if (x < 0 || y < 0 || x > w || y > h) {
    mStateFlags &= ~StateFlags::Hovered;

    MouseMoveEvent mme;
    static_cast<MouseEvent&>(mme) = *e;

    static constexpr auto invalid = -std::numeric_limits<SkScalar>::infinity();
    mme.mPoint = {-invalid, -invalid};

    for (auto&& child: this->GetChildren()) {
      child->DispatchEvent(&mme);
    }
    return;
  }
  mStateFlags |= StateFlags::Hovered;

  unique_ptr<MouseEvent> translated;
  const auto Translate
    = [&translated, point]<std::derived_from<MouseEvent> T>(const T* event) {
        translated.reset(new T(*event));
        translated->mPoint = point;
        return static_cast<const T&>(*translated);
      };

  if (const auto it = dynamic_cast<const MouseMoveEvent*>(e)) {
    Translate(it);
  }

  if (!translated) {
    return;
  }

  // Always propagate 1 level unconditionally, to allow children to synthesize
  // mouse enter/leave events
  for (auto&& child: this->GetChildren()) {
    child->DispatchMouseEvent(translated.get());
  }
}

}// namespace FredEmmott::GUI::Widgets