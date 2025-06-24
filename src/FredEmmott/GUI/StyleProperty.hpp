// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <yoga/YGValue.h>

#include <FredEmmott/GUI/detail/style_detail.hpp>
#include <FredEmmott/memory.hpp>

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

template <class T, auto TDefault, StylePropertyScope TDefaultScope>
class BaseStyleProperty : private std::optional<T> {
 public:
  static constexpr StylePropertyScope DefaultScope = TDefaultScope;
  static constexpr bool SupportsTransitions = Interpolation::lerpable<T>;
  static constexpr auto DefaultValue = TDefault;

  friend struct Style;

  using std::optional<T>::optional;
  using std::optional<T>::operator->;
  using std::optional<T>::operator*;
  using std::optional<T>::value;
  using std::optional<T>::has_value;
  using std::optional<T>::value_or;
  using value_type = typename std::optional<T>::value_type;

  constexpr auto value_or_default() const {
    return this->value_or(TDefault);
  }

  BaseStyleProperty(
    const T& value,
    const std::convertible_to<std::optional<StyleTransition>> auto& transition)
    requires SupportsTransitions
    : std::optional<T>(value),
      mTransition(transition) {
    if constexpr (std::same_as<T, float>) {
      if (YGFloatIsUndefined(value)) {
        static_cast<std::optional<T>&>(*this) = std::nullopt;
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
    return static_cast<const std::optional<T>&>(*this) == other;
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
      static_cast<std::optional<T>&>(*this) = other.value();
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
  FUI_NO_UNIQUE_ADDRESS optional_transition_t mTransition;
  bool mIsImportant {false};
};

template <class T, auto TDefault = style_detail::default_v<T>>
using StyleProperty = BaseStyleProperty<T, TDefault, StylePropertyScope::Self>;
template <class T, auto TDefault = style_detail::default_v<T>>
using InheritableStyleProperty
  = BaseStyleProperty<T, TDefault, StylePropertyScope::SelfAndDescendants>;
}// namespace FredEmmott::GUI
