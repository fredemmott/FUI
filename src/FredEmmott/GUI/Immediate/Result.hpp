// Copyright 2025 Fred Emmott<fred @fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate/ScopeableResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/StyledResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/ValueResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/widget_from_result.hpp>

namespace FredEmmott::GUI::Immediate {
template <void (*TEndWidget)() = nullptr, class TValue = void, class... TMixins>
class Result final : public immediate_detail::StyledResultMixin<TMixins...>,
                     public immediate_detail::ValueResultMixin<TValue>,
                     public immediate_detail::
                       ScopeableResultMixin<TEndWidget, TValue, TMixins...>,
                     public TMixins... {
 public:
  using value_type = TValue;
  template <class... TExtraMixins>
  using extended_type = Result<TEndWidget, TValue, TMixins..., TExtraMixins...>;
  using type = extended_type<>;

  static constexpr bool HasWidget
    = !(std::same_as<immediate_detail::WidgetlessResultMixin, TMixins> || ...);
  static constexpr bool HasValue = !std::is_void_v<TValue>;

  template <void (*)(), class, class...>
  friend class Result;
  friend struct immediate_detail::ValueResultMixin<TValue>;
  friend std::
    conditional_t<HasWidget, immediate_detail::widget_from_result_t, void>;

  Result() = delete;

  constexpr Result()
    requires(!HasValue && !HasWidget)
  = default;

  template <std::convertible_to<TValue> T>
    requires(HasValue && !HasWidget)
  constexpr Result(T&& value) {
    this->mValue = std::forward<T>(value);
  }

  constexpr Result(Widgets::Widget* widget)
    requires(HasWidget && !HasValue)
    : mWidget(widget) {};

  template <std::convertible_to<TValue> T>
    requires(HasWidget && HasValue)
  constexpr Result(Widgets::Widget* widget, T&& result) : mWidget(widget) {
    this->mValue = std::forward<T>(result);
  }

  template <void (*TOtherEndWidget)(), class... TOtherMixins>
  constexpr Result(
    const Result<TOtherEndWidget, TValue, TOtherMixins...>& other)
    : mWidget(other.mWidget) {
    if constexpr (HasValue) {
      this->mValue = other.mValue;
    }
  }

  template <
    std::convertible_to<TValue> T,
    void (*TOtherEndWidget)(),
    class... TOtherMixins>
    requires(
      HasWidget && HasValue
      && Result<TOtherEndWidget, void, TOtherMixins...>::HasWidget)
  constexpr Result(
    const Result<TOtherEndWidget, void, TOtherMixins...>& other,
    T&& result)
    : mWidget(other.mWidget) {
    this->mValue = std::forward<T>(result);
  }

 private:
  std::conditional_t<HasWidget, Widgets::Widget*, std::monostate> mWidget {};
};
}// namespace FredEmmott::GUI::Immediate