// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <utility>

namespace FredEmmott::GUI {
// Like an std::atomic_flag, but not atomic :D
class ActivatedFlag {
 public:
  void Set() {
    mValue = true;
  }

  [[nodiscard]]
  bool TestAndClear() {
    return std::exchange(mValue, false);
  }

 private:
  bool mValue {false};
};
}// namespace FredEmmott::GUI