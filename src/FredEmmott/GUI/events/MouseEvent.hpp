// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkPoint.h>

#include "Event.hpp"
#include "MouseButton.hpp"

namespace FredEmmott::GUI {

struct MouseEvent : Event {
  SkPoint mPoint {};
  MouseButtons mButtons {};
};

}// namespace FredEmmott::GUI