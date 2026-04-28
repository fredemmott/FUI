// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Point.hpp>
#include <cstdint>
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

  template <class T>
  const auto& Get() const {
    return std::get<T>(mDetail);
  }

  struct MoveEvent {};
  // The mouse is staying relatively still, within an OS-defined bounding box
  // for an OS-defined amount of time
  struct HoverEvent {};
  struct ButtonPressEvent {
    MouseButtons mPressedButtons {};
    // 1 for a single click, 2 for the second click of a double-click, 3 for
    // the third click of a triple-click, etc. The platform layer is
    // responsible for the OS-defined click-time and pixel-drift thresholds —
    // SDL3 surfaces this as SDL_MouseButtonEvent::clicks; Win32 has
    // WM_LBUTTONDBLCLK + manual triple-click tracking.
    std::uint8_t mClickCount {1};
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
    HoverEvent,
    ButtonPressEvent,
    ButtonReleaseEvent,
    HorizontalWheelEvent,
    VerticalWheelEvent>
    mDetail {};
};

}// namespace FredEmmott::GUI
