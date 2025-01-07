// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class StackPanel final : public Widget {
 public:
  enum class Direction {
    Horizontal,
    Vertical,
  };
  StackPanel(std::size_t id, Direction);

 protected:
  WidgetStyles GetBuiltInStyles() const override;
  Direction mDirection;
};

}// namespace FredEmmott::GUI::Widgets