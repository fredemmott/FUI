// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/YGValue.h>

#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/style_detail.hpp>

#include "StaticTheme/Resource.hpp"
#include "StaticTheme/detail/ResolveColor.hpp"
#include "StyleTransition.hpp"

namespace FredEmmott::GUI {

struct Style;
class ImmutableStyle;

struct important_style_property_t {
  constexpr const important_style_property_t& operator!() const noexcept {
    return *this;
  }
};
constexpr important_style_property_t important;

template <class T>
class StyleProperty {
 public:
  using value_type = T;
  using resource_type = T (*)(StaticTheme::Theme);

  static constexpr bool SupportsTransitions = Interpolation::lerpable<T>;

  friend struct Style;
  friend class ImmutableStyle;

  constexpr StyleProperty() = default;
  constexpr StyleProperty(std::nullopt_t) {};
  StyleProperty(std::nullptr_t) = delete;

  template <class U = std::remove_cv_t<T>>
    requires std::is_constructible_v<T, U>
    && (!std::convertible_to<U, resource_type>)
    && (!std::same_as<std::remove_cvref_t<U>, StyleProperty>)
    && (!std::same_as<std::remove_cvref_t<U>, resource_type>)
  explicit(!std::is_convertible_v<U, T>) constexpr StyleProperty(
    U&& v) noexcept(std::is_nothrow_constructible_v<T, U>)
    : mValue(std::in_place_type<T>, std::forward<U>(v)) {}

  template <class U>
    requires requires(StaticTheme::Theme t) { StaticTheme::ResolveAs<T, U>(t); }
  constexpr StyleProperty(const U)
    : mValue(std::in_place_type<resource_type>, &StaticTheme::ResolveAs<T, U>) {
  }

  constexpr T value() const {
    if (const auto it = get_if<T>(&mValue)) {
      return *it;
    }
    if (const auto it = get_if<resource_type>(&mValue)) {
      return (*it)(StaticTheme::GetCurrent());
    }
    throw std::bad_variant_access();
  }

  constexpr bool has_value() const noexcept {
    return !holds_alternative<std::monostate>(mValue);
  }

  T operator->() const {
    return value();
  }

  [[nodiscard]]
  T operator*() const {
    return value();
  }

  template <std::convertible_to<T> U>
  constexpr T value_or(U&& v) const {
    if (has_value()) {
      return value();
    }
    return std::forward<U>(v);
  }

  constexpr operator std::optional<T>() const {
    if (has_value()) {
      return value();
    }
    return std::nullopt;
  }

  template <class U>
  StyleProperty(
    U&& u,
    const std::convertible_to<std::optional<StyleTransition>> auto& transition)
    requires SupportsTransitions
    && requires { StyleProperty(std::forward<U>(u)); }
    : StyleProperty(std::forward<U>(u)) {
    mTransition = transition;
  }

  template <class U>
  StyleProperty(U&& u, important_style_property_t)
    requires requires { StyleProperty(std::forward<U>(u)); }
    : StyleProperty(std::forward<U>(u)) {
    mPriority = StylePropertyPriority::Important;
  }

  template <class U, class V>
  StyleProperty(U&& u, V&& v, important_style_property_t)
    requires requires { StyleProperty(std::forward<U>(u), std::forward<V>(v)); }
    : StyleProperty(std::forward<U>(u), std::forward<V>(v)) {
    mPriority = StylePropertyPriority::Important;
  }

  [[nodiscard]]
  constexpr bool has_transition() const noexcept
    requires SupportsTransitions
  {
    return mTransition.has_value();
  }

  [[nodiscard]]
  constexpr decltype(auto) transition() const
    requires SupportsTransitions
  {
    return mTransition.value();
  }
  constexpr bool operator==(const StyleProperty& other) const noexcept
    = default;
  constexpr bool operator==(const T& other) const noexcept {
    if (!has_value()) {
      return false;
    }
    return value() == other;
  }

  constexpr operator bool() const noexcept {
    return this->has_value();
  }

  constexpr StyleProperty& operator+=(const StyleProperty& other) noexcept {
    if (std::to_underlying(mPriority) > std::to_underlying(other.mPriority)) {
      return *this;
    }

    if (other.has_value()) {
      mValue = other.mValue;
      mScope = other.mScope;
      mPriority = other.mPriority;
    }
    if constexpr (SupportsTransitions) {
      if (other.has_transition()) {
        mTransition = other.transition();
      }
    }
    return *this;
  }

  constexpr StyleProperty operator+(const StyleProperty& other) const noexcept {
    StyleProperty ret {*this};
    ret += other;
    return ret;
  }

 private:
  using optional_transition_t = std::conditional_t<
    SupportsTransitions,
    std::optional<StyleTransition>,
    std::monostate>;

  std::optional<StylePropertyScope> mScope;
  std::variant<std::monostate, T, resource_type> mValue {};
  StylePropertyPriority mPriority {StylePropertyPriority::Normal};
  optional_transition_t mTransition;
};

}// namespace FredEmmott::GUI
