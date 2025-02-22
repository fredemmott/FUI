// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPoint.h>

#include <variant>

#include "Event.hpp"
#include "MouseButton.hpp"

namespace FredEmmott::GUI {

struct MouseEvent final : Event {
  SkPoint mWindowPoint {};
  SkPoint mOffset {};
  MouseButtons mButtons {};

  [[nodiscard]]
  bool IsValid() const {
    return mWindowPoint.x() >= 0 && mWindowPoint.y() >= 0;
  }

  SkPoint GetPosition() const {
    return mWindowPoint + mOffset;
  }

  MouseEvent WithOffset(const SkPoint& offset) const {
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
  std::variant<MoveEvent, ButtonPressEvent, ButtonReleaseEvent> mDetail {};
};

}// namespace FredEmmott::GUI