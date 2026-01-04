// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/assert.hpp>
#include <bit>
#include <compare>

namespace FredEmmott::GUI {

struct NativeWaitable {
#ifdef _WIN32
  // HANDLE
  void* mHandle {nullptr};
#else
  int mFD {-1};
#endif

  using value_type = decltype(mHandle);
  constexpr auto operator<=>(const NativeWaitable&) const = default;

  NativeWaitable() = delete;

  explicit constexpr NativeWaitable(value_type value) {
#ifdef _WIN32
    FUI_ASSERT(value, "Can't create empty NativeWaitable");
    FUI_ASSERT(
      std::bit_cast<intptr_t>(value) != -1,
      "Can't create a NativeWaitable for INVALID_HANDLE_VALUE");
    mHandle = value;
#else
    FUI_ASSERT(value >= 0, "NativeWaitable FD is invalid");
    mFD = value;
#endif
  }
};

}// namespace FredEmmott::GUI