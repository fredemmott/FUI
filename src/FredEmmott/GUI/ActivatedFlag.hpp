// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <utility>

namespace FredEmmott::GUI {
// Like an std::atomic_flag, but not atomic :D
class ActivatedFlag {
 public:
  void Activate() {
    mActivated = true;
  }

  [[nodiscard]]
  bool TestAndClear() {
    return std::exchange(mActivated, false);
  }

 private:
  bool mActivated {false};
};
}// namespace FredEmmott::GUI