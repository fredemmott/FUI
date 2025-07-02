// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::style_detail {

struct lazy_init_style {
  using InitFun = Style (*)();
  lazy_init_style() = delete;
  explicit constexpr lazy_init_style(InitFun fn) : mFn(fn) {}

  operator const Style&() const {
    std::call_once(sOnce, [this]() { mStyle = mFn(); });
    return mStyle;
  }

 private:
  InitFun mFn {nullptr};
  mutable std::once_flag sOnce;
  mutable Style mStyle;
};

}// namespace FredEmmott::GUI::style_detail