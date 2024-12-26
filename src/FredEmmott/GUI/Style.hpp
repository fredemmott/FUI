// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>
#include <YGValue.h>

#include <chrono>
#include <optional>

#include "Brush.hpp"
#include "Font.hpp"
#include "FredEmmott/memory.hpp"
#include "StyleTransitions.hpp"

namespace FredEmmott::GUI {

template <class T>
struct Transition {
  using optional_type = Transition;

  constexpr bool has_value() const noexcept {
    return false;
  }

  [[noreturn]]
  constexpr T value() const {
    throw std::bad_optional_access {};
  }

  constexpr bool operator==(const Transition&) const noexcept = default;
};

template <animatable T>
struct Transition<T> {
  using optional_type = std::optional<Transition>;

  Transition() = delete;
  Transition(const LinearStyleTransition<T>& value) : mValue(value) {
  }
  Transition(const CubicBezierStyleTransition<T>& value) : mValue(value) {
  }

  std::chrono::milliseconds GetDuration() const {
    if (holds_alternative<LinearStyleTransition<T>>(mValue)) {
      return get<LinearStyleTransition<T>>(mValue).GetDuration();
    }
    if (holds_alternative<CubicBezierStyleTransition<T>>(mValue)) {
      return get<CubicBezierStyleTransition<T>>(mValue).GetDuration();
    }
    throw std::bad_variant_access {};
  }

  T Evaluate(SkScalar normalizedX) const {
    if (const auto it = get_if<LinearStyleTransition<T>>(&mValue)) {
      return it->Evaluate(normalizedX);
    }
    if (const auto it = get_if<CubicBezierStyleTransition<T>>(&mValue)) {
      return it->Evaluate(normalizedX);
    }
    throw std::bad_variant_access {};
  }

  constexpr bool operator==(const Transition&) const noexcept = default;

 private:
  std::variant<LinearStyleTransition<T>, CubicBezierStyleTransition<T>> mValue;
};

class Style;
class WidgetStyles;

enum class StyleValueScope {
  Self,
  SelfAndChildren,
  SelfAndDescendants,
};

template <class T, StyleValueScope TDefaultScope = StyleValueScope::Self>
class StyleValue : public std::optional<T> {
 public:
  static constexpr StyleValueScope DefaultScope = TDefaultScope;

  friend class Style;
  friend class WidgetStyles;

  using std::optional<T>::optional;
  StyleValue(
    const T& value,
    const std::convertible_to<Transition<T>> auto& transition)
    requires animatable<T>
    : std::optional<T>(value), mTransition(transition) {
    if (std::same_as<T, SkScalar> && YGFloatIsUndefined(value)) {
      static_cast<std::optional<T>&>(*this) = std::nullopt;
    }
  }

  StyleValue(
    std::nullopt_t,
    const std::convertible_to<Transition<T>> auto& transition)
    requires animatable<T>
    : StyleValue(YGUndefined, transition) {
  }

  [[nodiscard]]
  constexpr bool has_transition() const noexcept
    requires animatable<T>
  {
    return mTransition.has_value();
  }

  [[nodiscard]]
  constexpr decltype(auto) transition() const
    requires animatable<T>
  {
    return mTransition.value();
  }
  constexpr bool operator==(const StyleValue& other) const noexcept = default;

  constexpr StyleValue& operator+=(const StyleValue& other) noexcept {
    if (other.has_value()) {
      static_cast<std::optional<T>&>(*this) = other;
      mScope = other.mScope;
    }
    if constexpr (animatable<T>) {
      if (other.has_transition()) {
        mTransition = other.transition();
      }
    }
    return *this;
  }

  constexpr StyleValue operator+(const StyleValue& other) const noexcept {
    StyleValue ret {*this};
    ret += other;
    return ret;
  }

 private:
  StyleValueScope mScope {TDefaultScope};
  FUI_NO_UNIQUE_ADDRESS
  typename Transition<T>::optional_type mTransition;
};

template <class T>
struct InheritableStyleValue
  : StyleValue<T, StyleValueScope::SelfAndDescendants> {
  constexpr bool operator==(const InheritableStyleValue& other) const noexcept
    = default;
};

struct Style {
  StyleValue<YGAlign> mAlignSelf;
  StyleValue<YGAlign> mAlignItems;
  StyleValue<Brush> mBackgroundColor;
  StyleValue<Brush> mBorderColor;
  StyleValue<SkScalar> mBorderRadius;
  StyleValue<SkScalar> mBorderWidth;
  StyleValue<SkScalar> mBottom;
  InheritableStyleValue<Brush> mColor;
  StyleValue<YGDisplay> mDisplay;
  StyleValue<YGFlexDirection> mFlexDirection;
  InheritableStyleValue<Font> mFont;
  StyleValue<SkScalar> mGap;
  StyleValue<SkScalar> mHeight;
  StyleValue<SkScalar> mLeft;
  StyleValue<SkScalar> mMargin;
  StyleValue<SkScalar> mMarginBottom;
  StyleValue<SkScalar> mMarginLeft;
  StyleValue<SkScalar> mMarginRight;
  StyleValue<SkScalar> mMarginTop;
  StyleValue<SkScalar> mPadding;
  StyleValue<SkScalar> mPaddingBottom;
  StyleValue<SkScalar> mPaddingLeft;
  StyleValue<SkScalar> mPaddingRight;
  StyleValue<SkScalar> mPaddingTop;
  StyleValue<SkScalar> mRight;
  StyleValue<SkScalar> mTop;
  StyleValue<SkScalar> mWidth;

  [[nodiscard]] Style InheritableValues() const noexcept;

  Style& operator+=(const Style& other) noexcept;
  Style operator+(const Style& other) const noexcept {
    Style ret {*this};
    ret += other;
    return ret;
  }

  bool operator==(const Style& other) const noexcept = default;
};

}// namespace FredEmmott::GUI

#define FUI_STYLE_PROPERTIES(X) \
  X(AlignItems) \
  X(AlignSelf) \
  X(BackgroundColor) \
  X(BorderColor) \
  X(BorderRadius) \
  X(BorderWidth) \
  X(Bottom) \
  X(Color) \
  X(Display) \
  X(Font) \
  X(FlexDirection) \
  X(Gap) \
  X(Height) \
  X(Left) \
  X(Margin) \
  X(MarginBottom) \
  X(MarginLeft) \
  X(MarginRight) \
  X(MarginTop) \
  X(Padding) \
  X(PaddingBottom) \
  X(PaddingLeft) \
  X(PaddingRight) \
  X(PaddingTop) \
  X(Right) \
  X(Top) \
  X(Width)
