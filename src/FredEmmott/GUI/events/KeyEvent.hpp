// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Event.hpp"
#include "KeyCode.hpp"

namespace FredEmmott::GUI {

struct KeyEvent : Event {
  KeyEvent() = delete;
  constexpr explicit KeyEvent(const KeyCode keyCode) : mKeyCode(keyCode) {}
  ~KeyEvent() override = default;

  KeyCode mKeyCode;
};

struct KeyPressEvent final : KeyEvent {
  ~KeyPressEvent() override = default;

  using KeyEvent::KeyEvent;
};

struct KeyReleaseEvent final : KeyEvent {
  ~KeyReleaseEvent() override = default;

  using KeyEvent::KeyEvent;
};


}