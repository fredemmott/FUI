// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class Card final : public Widget {
 public:
  Card();
  ~Card() override;
};

}// namespace FredEmmott::GUI::Widgets