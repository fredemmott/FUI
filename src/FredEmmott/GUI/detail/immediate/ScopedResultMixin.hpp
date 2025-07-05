// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/utility/moved_flag.hpp>
#include <stdexcept>
#include <utility>

namespace FredEmmott::GUI::Immediate::immediate_detail {
template <void (*TEndWidget)()>
struct ScopedEndWidget {
  ScopedEndWidget(const ScopedEndWidget&) = delete;
  ScopedEndWidget& operator=(const ScopedEndWidget&) = delete;

  constexpr ScopedEndWidget() = default;
  constexpr ScopedEndWidget(ScopedEndWidget&&) noexcept = default;
  constexpr ScopedEndWidget& operator=(ScopedEndWidget&&) noexcept = default;

  constexpr ~ScopedEndWidget() {
    if (!mMoved) {
      TEndWidget();
    }
  }

 private:
  utility::moved_flag mMoved;
};

template <void (*TEndWidget)()>
struct ScopedResultMixin {
  auto Scoped() {
    if (std::exchange(mScoped, true)) [[unlikely]] {
      throw std::logic_error("Can't call Scoped() twice on the same Result");
    }
    return ScopedEndWidget<TEndWidget> {};
  }

 private:
  bool mScoped = false;
};
template <>
struct ScopedResultMixin<nullptr> {};
}// namespace FredEmmott::GUI::Immediate::immediate_detail
