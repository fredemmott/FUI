// Copyright 2025 Fred Emmott<fred @fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/immediate/ScopedResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/StyledResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/ValueResultMixin.hpp>
#include <FredEmmott/GUI/detail/immediate/widget_from_result.hpp>

namespace FredEmmott::GUI::Immediate {
template <void (*TEndWidget)() = nullptr, class TValue = void, class... TMixins>
class Result final : public immediate_detail::StyledResultMixin<TMixins...>,
                     public immediate_detail::ScopedResultMixin<TEndWidget>,
                     public immediate_detail::ValueResultMixin<TValue>,
                     public TMixins... {
 public:
  static constexpr bool HasWidget
    = !(std::same_as<immediate_detail::WidgetlessResultMixin, TMixins> || ...);
  static constexpr bool HasValue = !std::is_void_v<TValue>;

  template <void (*)(), class, class...>
  friend class Result;
  friend struct immediate_detail::ValueResultMixin<TValue>;
  friend std::conditional_t<
    HasWidget,
    immediate_detail::widget_from_result_t<Result>,
    void>;

  Result() = delete;

  Result()
    requires(!HasValue && !HasWidget)
  = default;

  template <std::convertible_to<TValue> T>
    requires(HasValue && !HasWidget)
  constexpr Result(T&& result) : mValue(std::forward<T>(result)) {}

  constexpr Result(Widgets::Widget* widget)
    requires(HasWidget && !HasValue)
    : mWidget(widget) {};

  template <void (*TOtherEndWidget)(), class... TOtherMixins>
    requires(
      HasWidget && !HasValue
      && Result<TOtherEndWidget, void, TOtherMixins...>::HasWidget)
  constexpr Result(const Result<TOtherEndWidget, void, TOtherMixins...>& other)
    : mWidget(other.mWidget) {}

  template <std::convertible_to<TValue> T>
    requires(HasWidget && HasValue)
  constexpr Result(Widgets::Widget* widget, T&& result)
    : mWidget(widget),
      mValue(std::forward<T>(result)) {}

  template <
    std::convertible_to<TValue> T,
    void (*TOtherEndWidget)(),
    class... TOtherMixins>
    requires(HasWidget && HasValue
             && Result<TOtherEndWidget, void, TOtherMixins...>::HasWidget)
  constexpr Result(
    const Result<TOtherEndWidget, void, TOtherMixins...>& other,
    T&& result)
    : mWidget(other.mWidget),
      mValue(std::forward<T>(result)) {}

 private:
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<HasWidget, Widgets::Widget*, std::monostate> mWidget {};
  FUI_NO_UNIQUE_ADDRESS
  std::conditional_t<HasValue, TValue, std::monostate> mValue;
};
}// namespace FredEmmott::GUI::Immediate