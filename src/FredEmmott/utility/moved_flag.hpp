// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::utility {

struct moved_flag {
  constexpr moved_flag() = default;
  moved_flag(const moved_flag&) = delete;
  moved_flag& operator=(const moved_flag&) = delete;

  constexpr moved_flag(moved_flag&& other) noexcept {
    other.mMoved = true;
  }

  constexpr moved_flag& operator=(moved_flag&& other) noexcept {
    other.mMoved = false;
    return *this;
  }

  constexpr operator bool() const noexcept {
    return mMoved;
  }

 private:
  bool mMoved = false;
};

}// namespace FredEmmott::utility