// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Point.hpp>
#include <optional>
#include <variant>

#include "Event.hpp"
#include "MouseButton.hpp"

namespace FredEmmott::GUI {

struct MouseEvent final : Event {
  // Pen / stylus axes when the event came from a pressure-sensitive device.
  // `mPenAxes.has_value()` is the "this is a pen" signal — existing widgets
  // ignore it and treat the event as an ordinary mouse event; OpenKneeboard
  // ink-aware widgets read pressure / tilt / eraser from here. All axes are
  // optional inside the struct because not every device reports every axis.
  // Ranges follow SDL3's SDL_PenAxis: pressure/distance unidirectional 0..1,
  // tiltX/tiltY in degrees -90..90, rotation in degrees -180..180.
  struct PenAxes {
    std::optional<float> mPressure;
    std::optional<float> mTiltX;
    std::optional<float> mTiltY;
    std::optional<float> mDistance;
    std::optional<float> mRotation;
    bool mEraser {false};
  };

  Point mWindowPoint {};
  Point mOffset {};
  MouseButtons mButtons {};
  std::optional<PenAxes> mPenAxes;

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
