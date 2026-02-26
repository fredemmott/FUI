// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Orientation.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class StackPanel final : public Widget {
 public:
  StackPanel(id_type id, Orientation);
};

}// namespace FredEmmott::GUI::Widgets