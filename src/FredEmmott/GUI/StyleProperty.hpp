// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/YGValue.h>

#include <FredEmmott/GUI/config.hpp>
#include <FredEmmott/GUI/detail/style_detail.hpp>

#include "StyleTransition.hpp"

namespace FredEmmott::GUI {

struct Style;

enum class StylePropertyScope {
  Self,
  SelfAndChildren,
  SelfAndDescendants,
};

struct important_style_property_t {
  constexpr const important_style_property_t& operator!() const noexcept {
    return *this;
  }
};
constexpr important_style_property_t important;

template <
  class T,
  StylePropertyScope TDefaultScope,
  auto TDefault = style_detail::default_v<T>>
class BaseStyleProperty {
 public:
  using default_type = std::decay_t<decltype(TDefault)>;
  using value_type = T;
  using resource_type = const StaticTheme::Resource<T>*;

  static constexpr StylePropertyScope DefaultScope = TDefaultScope;
  static constexpr bool SupportsTransitions = Interpolation::lerpable<T>;
  static constexpr auto DefaultValue = TDefault;

  friend struct Style;

  constexpr BaseStyleProperty() = default;
  constexpr BaseStyleProperty(std::nullopt_t) {};
  BaseStyleProperty(std::nullptr_t) = delete;

  template <class U = std::remove_cv_t<T>>
    requires std::is_constructible_v<T, U>
    && (!std::convertible_to<U, resource_type>)
    && (!std::same_as<std::remove_cvref_t<U>, BaseStyleProperty>)
    && (!std::same_as<std::remove_cvref_t<U>, resource_type>)
  explicit(!std::is_convertible_v<U, T>) constexpr BaseStyleProperty(
    U&& v) noexcept(std::is_nothrow_constructible_v<T, U>)
    : mValue(std::in_place_type<T>, std::forward<U>(v)) {}
  template <std::convertible_to<resource_type> U>
    requires(!std::same_as<std::nullptr_t, std::decay_t<U>>)
  BaseStyleProperty(U&& r)
    : mValue(std::in_place_type<resource_type>, std::forward<U>(r)) {}

  constexpr const T& value() const {
    if (const auto it = get_if<T>(&mValue)) {
      return *it;
    }
    if (const auto it = get_if<resource_type>(&mValue)) {
      return *(*it)->Resolve();
    }
    throw std::bad_variant_access();
  }

  constexpr bool has_value() const noexcept {
    return !holds_alternative<std::monostate>(mValue);
  }

  const T* operator->() const {
    return &value();
  }

  const T& operator*() const {
    return value();
  }

  template <std::convertible_to<T> U>
  constexpr T value_or(U&& v) const {
    if (has_value()) {
      return value();
    }
    return std::forward<U>(v);
  }

  constexpr auto value_or_default() const {
    return this->value_or(TDefault);
  }

  BaseStyleProperty(
    const T& value,
    const std::convertible_to<std::optional<StyleTransition>> auto& transition)
    requires SupportsTransitions
    : mValue(std::in_place_type<T>, value),
      mTransition(transition) {
    if constexpr (std::same_as<T, float>) {
      if (YGFloatIsUndefined(value)) {
        mValue = {};
      }
    }
  }

  BaseStyleProperty(
    std::nullopt_t,
    const std::convertible_to<std::optional<StyleTransition>> auto& transition)
    requires SupportsTransitions && std::same_as<T, float>
    : mTransition(transition) {}

  BaseStyleProperty(
    std::nullopt_t,
    const std::convertible_to<std::optional<StyleTransition>> auto& transition)
    requires SupportsTransitions && (!std::same_as<T, float>)
    : mTransition(transition) {}

  template <class U>
  BaseStyleProperty(U&& u, important_style_property_t)
    requires requires { BaseStyleProperty(std::forward<U>(u)); }
    : BaseStyleProperty(std::forward<U>(u)) {
    mIsImportant = true;
  }

  template <class U, class V>
  BaseStyleProperty(U&& u, V&& v, important_style_property_t)
    requires requires {
      BaseStyleProperty(std::forward<U>(u), std::forward<V>(v));
    }
    : BaseStyleProperty(std::forward<U>(u), std::forward<V>(v)) {
    mIsImportant = true;
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
  constexpr bool operator==(const BaseStyleProperty& other) const noexcept
    = default;
  constexpr bool operator==(const T& other) const noexcept {
    if (!has_value()) {
      if constexpr (std::same_as<std::nullopt_t, default_type>) {
        return false;
      } else {
        return TDefault == other;
      }
    }
    return value() == other;
  }

  constexpr operator bool() const noexcept {
    return this->has_value();
  }

  constexpr BaseStyleProperty& operator+=(
    const BaseStyleProperty& other) noexcept {
    if (mIsImportant && !other.mIsImportant) {
      return *this;
    }
    mIsImportant = other.mIsImportant;
    if (other.has_value()) {
      mValue = other.mValue;
      mScope = other.mScope;
    }
    if constexpr (SupportsTransitions) {
      if (other.has_transition()) {
        mTransition = other.transition();
      }
    }
    return *this;
  }

  constexpr BaseStyleProperty operator+(
    const BaseStyleProperty& other) const noexcept {
    BaseStyleProperty ret {*this};
    ret += other;
    return ret;
  }

 private:
  using optional_transition_t = std::conditional_t<
    SupportsTransitions,
    std::optional<StyleTransition>,
    std::monostate>;

  StylePropertyScope mScope {TDefaultScope};
  std::variant<std::monostate, T, resource_type> mValue {};
  bool mIsImportant {false};
  optional_transition_t mTransition;
};

template <class T, auto TDefault = style_detail::default_v<T>>
using StyleProperty = BaseStyleProperty<T, StylePropertyScope::Self, TDefault>;
template <class T, auto TDefault = style_detail::default_v<T>>
using InheritableStyleProperty
  = BaseStyleProperty<T, StylePropertyScope::SelfAndDescendants, TDefault>;
}// namespace FredEmmott::GUI
