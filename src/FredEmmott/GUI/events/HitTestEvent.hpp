// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Point.hpp>

#include "Event.hpp"

namespace FredEmmott::GUI {

struct HitTestEvent final : public Event {
  Point mPoint {};
  HitTestEvent WithOffset(const Point& offset) {
    HitTestEvent ret {*this};
    ret.mPoint += offset;
    return ret;
  }
};

}// namespace FredEmmott::GUI