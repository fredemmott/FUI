// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI {

struct Event {
  // allow dynamic_cast
  virtual ~Event() = default;
};

}// namespace FredEmmott::GUI