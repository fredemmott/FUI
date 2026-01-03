// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Point.hpp>

#include "Event.hpp"

namespace FredEmmott::GUI {

struct HitTestEvent final : Event {
  Point mPoint {};
  constexpr HitTestEvent WithOffset(const Point& offset) const noexcept {
    HitTestEvent ret {*this};
    ret.mPoint += offset;
    return ret;
  }
};

}// namespace FredEmmott::GUI