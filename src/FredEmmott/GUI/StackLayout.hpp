// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Layout.hpp"

namespace FredEmmott::GUI {

class StackLayout final : public Layout {
  public:
   struct Options {};
   enum class Direction {
     Horizontal,
     Vertical,
   };
   StackLayout(std::size_t id, const Options&, Direction);
};

}