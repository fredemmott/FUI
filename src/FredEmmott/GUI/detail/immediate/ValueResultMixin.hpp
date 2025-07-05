// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::Immediate::immediate_detail {

template <class TValue>
struct ValueResultMixin {
  constexpr TValue GetValue(this const auto& self) noexcept {
    return self.mValue;
  }

  constexpr operator TValue(this const auto& self) noexcept {
    return self.GetValue();
  }
};
template <>
struct ValueResultMixin<void> {};

}// namespace FredEmmott::GUI::Immediate::immediate_detail
