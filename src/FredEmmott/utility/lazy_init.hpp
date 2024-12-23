// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <concepts>
#include <mutex>

namespace FredEmmott::utility {

template <class T>
class lazy_init {
 public:
  using TInit = T (*)();
  lazy_init() = delete;
  constexpr lazy_init(std::convertible_to<TInit> auto init) : mInit(init) {
  }

  const T& Get() const {
    std::call_once(mInitOnceFlag, [this]() { mValue = mInit(); });
    return mValue;
  }

 private:
  TInit mInit {};
  mutable T mValue {};
  mutable std::once_flag mInitOnceFlag;
};

}// namespace FredEmmott::utility