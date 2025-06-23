// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Point.hpp>
#include <variant>

#include "Event.hpp"
#include "MouseButton.hpp"

namespace FredEmmott::GUI {

struct MouseEvent final : Event {
  Point mWindowPoint {};
  Point mOffset {};
  MouseButtons mButtons {};

  [[nodiscard]]
  bool IsValid() const {
    return mWindowPoint.mX >= 0 && mWindowPoint.mY >= 0;
  }

  Point GetPosition() const {
    return mWindowPoint + mOffset;
  }

  MouseEvent WithOffset(const Point& offset) const {
    MouseEvent ret {*this};
    ret.mOffset += offset;
    return ret;
  }

  using MoveEvent = std::monostate;
  struct ButtonPressEvent {
    MouseButtons mPressedButtons {};
  };
  struct ButtonReleaseEvent {
    MouseButtons mReleasedButtons {};
  };
  struct HorizontalWheelEvent {
    float mDelta {};
  };
  struct VerticalWheelEvent {
    float mDelta {};
  };
  std::variant<
    MoveEvent,
    ButtonPressEvent,
    ButtonReleaseEvent,
    HorizontalWheelEvent,
    VerticalWheelEvent>
    mDetail {};
};

}// namespace FredEmmott::GUI