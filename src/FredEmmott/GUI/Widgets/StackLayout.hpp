// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Layout.hpp"

namespace FredEmmott::GUI::Widgets {

class StackLayout final : public Layout {
 public:
  enum class Direction {
    Horizontal,
    Vertical,
  };
  StackLayout(std::size_t id, Direction);

 protected:
  WidgetStyles GetDefaultStyles() const override;
  Direction mDirection;
};

}// namespace FredEmmott::GUI::Widgets