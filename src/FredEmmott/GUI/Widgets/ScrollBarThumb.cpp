// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBarThumb.hpp"

#include "FredEmmott/GUI/StaticTheme/ScrollBar.hpp"
#include "FredEmmott/GUI/detail/Widget/ScrollBar.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace StaticTheme::ScrollBar;
using namespace ScrollBarDetail;

namespace {

constexpr LiteralStyleClass ScrollBarThumbStyleClass("ScrollBar/Thumb");

auto BaseThumbStyle() {
  using namespace PseudoClasses;
  return Style()
    .BackgroundColor(ScrollBarThumbFill)
    .BorderRadius(ScrollBarCornerRadius)
    .Height(std::nullopt, ContractAnimation)
    .Width(std::nullopt, ExpandAnimation)
    .And(Disabled, Style().BackgroundColor(ScrollBarThumbFillDisabled))
    .And(Hover, Style().BackgroundColor(ScrollBarThumbFillPointerOver));
}

auto& HorizontalThumbStyle() {
  static const ImmutableStyle ret {
    BaseThumbStyle()
      + Style()
          .Height(ScrollBarHorizontalThumbMinHeight)
          .Width(ScrollBarHorizontalThumbMinWidth),
  };
  return ret;
}

auto& VerticalThumbStyle() {
  static const ImmutableStyle ret {
    BaseThumbStyle()
      + Style()
          .Height(ScrollBarVerticalThumbMinHeight)
          .Width(ScrollBarVerticalThumbMinWidth),
  };
  return ret;
}

}// namespace

ScrollBarThumb::ScrollBarThumb(const Orientation o, std::size_t id)
  : Widget(
      id,
      (o == Orientation::Horizontal) ? HorizontalThumbStyle()
                                     : VerticalThumbStyle(),
      {ScrollBarThumbStyleClass}) {}

ScrollBarThumb::~ScrollBarThumb() = default;

Widget::EventHandlerResult ScrollBarThumb::OnMouseButtonPress(
  const MouseEvent& e) {
  if (this->IsDisabled() || !e.IsValid()) {
    return EventHandlerResult::Default;
  }
  (void)Widget::OnMouseButtonPress(e);

  mDragStart = e.mWindowPoint;
  this->StartMouseCapture();
  return EventHandlerResult::StopPropagation;
}

void ScrollBarThumb::OnDrag(std::function<void(Point*)> callback) {
  mOnDragCallback = callback;
}

ScrollBarThumb::EventHandlerResult ScrollBarThumb::OnMouseMove(
  const MouseEvent& e) {
  if (!mDragStart.has_value()) {
    return EventHandlerResult::Default;
  }

  Point delta {e.mWindowPoint - *mDragStart};
  if (mOnDragCallback) {
    mOnDragCallback(&delta);
  }
  *mDragStart += delta;
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult ScrollBarThumb::OnMouseButtonRelease(
  const MouseEvent& e) {
  if (this->IsDisabled() || !mDragStart.has_value()) {
    return EventHandlerResult::Default;
  }
  (void)Widget::OnMouseButtonRelease(e);

  mDragStart = std::nullopt;
  this->EndMouseCapture();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets