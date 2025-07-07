// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/utility/moved_flag.hpp>
#include <stdexcept>
#include <utility>

namespace FredEmmott::GUI::Immediate::immediate_detail {

/** Scoped() returns a no-op scope if the value is false.
 *
 * Requires `std::same_as<TValue, bool>`
 */
struct ConditionallyScopedResultMixin {};
struct UnscopedResultMixin {};

template <void (*TEndWidget)(), class TValue, class... TMixins>
struct ScopedEndWidget {
  template <std::convertible_to<TValue> T>
    requires(!std::is_void_v<TValue>)
  explicit constexpr ScopedEndWidget(T&& result)
    : mResult(std::forward<T>(result)) {}

  ScopedEndWidget(const ScopedEndWidget&) = delete;
  ScopedEndWidget& operator=(const ScopedEndWidget&) = delete;

  constexpr ScopedEndWidget()
    requires(std::is_void_v<TValue>)
  = default;
  constexpr ScopedEndWidget(ScopedEndWidget&&) noexcept = default;
  constexpr ScopedEndWidget& operator=(ScopedEndWidget&&) noexcept = default;

  constexpr ~ScopedEndWidget() {
    if constexpr ((std::same_as<TMixins, ConditionallyScopedResultMixin>
                   || ...)) {
      static_assert(std::same_as<TValue, bool>);
      if (!mResult) {
        return;
      }
    }
    if (!mMoved) {
      TEndWidget();
    }
  }

  constexpr operator TValue() const
    requires(!std::is_void_v<TValue>)
  {
    return mResult;
  }

  constexpr TValue GetValue() const
    requires(!std::is_void_v<TValue>)
  {
    return mResult;
  }

 private:
  utility::moved_flag mMoved;
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<std::is_void_v<TValue>, std::monostate, TValue> mResult;
};

template <void (*TEndWidget)(), class TValue, class... TMixins>
struct ScopedResultMixin {
  template <class Self>
  auto Scoped(this Self&& self) {
    if (std::exchange(self.mScoped, true)) [[unlikely]] {
      throw std::logic_error("Can't call Scoped() twice on the same Result");
    }
    if constexpr (std::is_void_v<TValue>) {
      return ScopedEndWidget<TEndWidget, void, TMixins...> {};
    } else {
      return ScopedEndWidget<TEndWidget, TValue, TMixins...> {self.GetValue()};
    }
  }

 private:
  bool mScoped = false;
};

template <class T, class... TMixins>
struct ScopedResultMixin<nullptr, T, TMixins...> {};

template <void (*TEndWidget)(), class T, class... TMixins>
  requires(std::same_as<UnscopedResultMixin, TMixins> || ...)
struct ScopedResultMixin<TEndWidget, T, TMixins...> {};
}// namespace FredEmmott::GUI::Immediate::immediate_detail
