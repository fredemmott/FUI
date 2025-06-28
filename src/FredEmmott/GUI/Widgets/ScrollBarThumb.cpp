// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBarThumb.hpp"

namespace FredEmmott::GUI::Widgets {
const auto ScrollBarThumbStyleClass = StyleClass::Make("ScrollBarThumb");

ScrollBarThumb::ScrollBarThumb(std::size_t id)
  : Widget(id, {ScrollBarThumbStyleClass}) {}

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