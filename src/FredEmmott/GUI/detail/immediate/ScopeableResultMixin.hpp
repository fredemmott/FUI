// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <felly/moved_flag.hpp>
#include <stdexcept>
#include <utility>

#include "ValueResultMixin.hpp"

namespace FredEmmott::GUI::Immediate::immediate_detail {

/** Scoped() returns a no-op scope if the value is false.
 *
 * Requires `std::same_as<TValue, bool>`
 */
struct [[nodiscard]] ConditionallyScopeableResultMixin {};
struct UnscopeableResultMixin {};

// For use as a base case with std::conditional_t
struct EmptyResultMixin {};

template <void (*TEndWidget)()>
struct [[nodiscard]] ScopedResultMixin : UnscopeableResultMixin {
  ScopedResultMixin() = default;

  ScopedResultMixin(const ScopedResultMixin&) = delete;
  ScopedResultMixin& operator=(const ScopedResultMixin&) = delete;
  ScopedResultMixin(ScopedResultMixin&&) = default;
  ScopedResultMixin& operator=(ScopedResultMixin&&) = default;

  constexpr ~ScopedResultMixin() {
    if (!mMoved) {
      TEndWidget();
    }
  }

 private:
  template <void (*)(), class, class...>
  friend struct ScopeableResultMixin;

  void ReleaseScopedEndWidget() {
    // We already have a 'set to disable' flag, so let's re-use it :)
    // C++ allows move elision when assigning to non-volatile lvalues, so,
    // mark it volatile
    [[maybe_unused]]
    const volatile auto discard = std::move(mMoved);
  }

  felly::moved_flag mMoved;
};

template <void (*TEndWidget)(), class TValue, class... TMixins>
struct [[nodiscard]] ScopeableResultMixin {
  template <class Self>
  decltype(auto) Scoped(this Self&& self)
    requires std::is_rvalue_reference_v<decltype(self)>
  {
    auto ret =
      typename Self::template extended_type<ScopedResultMixin<TEndWidget>> {
        std::forward<Self>(self)};
    if constexpr ((std::same_as<TMixins, ConditionallyScopeableResultMixin>
                   || ...)) {
      if (!ret.GetValue()) {
        ret.ReleaseScopedEndWidget();
      }
    }
    return ret;
  }
};

template <class T, class... TMixins>
struct ScopeableResultMixin<nullptr, T, TMixins...> {};

template <void (*TEndWidget)(), class T, class... TMixins>
  requires(std::derived_from<TMixins, UnscopeableResultMixin> || ...)
struct [[nodiscard]] ScopeableResultMixin<TEndWidget, T, TMixins...> {};
}// namespace FredEmmott::GUI::Immediate::immediate_detail
