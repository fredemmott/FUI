// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <stdexcept>
#include <thread>

#include "FredEmmott/GUI/assert.hpp"

namespace FredEmmott::GUI::detail {

class AutomationActivityFlag {
 private:
  std::atomic<std::thread::id> mOwner {};
  std::size_t mDepth {};

 public:
  [[nodiscard]] bool try_lock() {
    const auto self = std::this_thread::get_id();

    if (mOwner.load(std::memory_order_relaxed) == self) {
      FUI_ASSERT(mDepth > 0);
      ++mDepth;
      return true;
    }

    std::thread::id expected {};
    if (!mOwner.compare_exchange_strong(
          expected, self, std::memory_order_acquire)) {
      return false;
    }
    FUI_ASSERT(mDepth == 0);
    mDepth = 1;
    return true;
  }

  void lock() {
    if (!try_lock()) {
      throw std::logic_error(
        "Tried to acquire a non-concurrent flag multiple times");
    }
  }

  void unlock() {
    FUI_ASSERT(mDepth > 0);
    FUI_ASSERT(
      mOwner.load(std::memory_order_relaxed) == std::this_thread::get_id());

    if (--mDepth == 0) {
      mOwner.store({}, std::memory_order_release);
    }
  }

  [[nodiscard]]
  bool test() const noexcept {
    const auto owner = mOwner.load(std::memory_order_relaxed);
    if (owner == std::thread::id {}) {
      return false;
    }
    // At least with Text Services Framework, everything should be in the
    // main thread.
    //
    // If this ends up being used with multithreaded frameworks, locking
    // will be needed to avoid race conditions
    FUI_ASSERT(owner == std::this_thread::get_id());
    return true;
  }

  operator bool() const noexcept {
    return test();
  }
};

}// namespace FredEmmott::GUI::detail