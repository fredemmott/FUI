// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/SystemSettings.hpp>
#include <FredEmmott/GUI/assert.hpp>
#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/widget_detail.hpp>
#include <FredEmmott/utility/almost_equal.hpp>
#include <print>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;
using utility::almost_equal;

namespace {
constexpr bool DebugAnimations = Config::Debug && Config::LibraryDeveloper;
constexpr bool SlowAnimations = false;

template <class T>
struct transition_default_value_t : constant_t<std::nullopt> {};
template <>
struct transition_default_value_t<float> : constant_t<0> {};
template <>
struct transition_default_value_t<Brush>
  : constant_t<[] constexpr { return Colors::Transparent; }> {};

template <class T>
constexpr auto transition_default_value_v
  = transition_default_value_t<T>::value;

template <class T>
struct FakeCopy {
  FakeCopy() = delete;
  explicit FakeCopy(const T&) {}
};
template <class T>
using DebugCopy = std::conditional_t<DebugAnimations, T, FakeCopy<T>>;

}// namespace

template <auto TStyleProperty, auto TStateProperty>
Widget::StyleTransitions::ApplyResult Widget::StyleTransitions::Apply(
  std::chrono::steady_clock::time_point now,
  const Style& oldStyle,
  Style* newStyle)
  requires(supports_transitions_v<TStyleProperty>)
{
  using enum ApplyResult;
  using TValue =
    typename std::decay_t<decltype(oldStyle.*TStyleProperty)>::value_type;
  constexpr auto DefaultValue = transition_default_value_v<TValue>;

  const auto oldProp = (oldStyle.*TStyleProperty);
  auto& newProp = (newStyle->*TStyleProperty);

  if (!newProp.has_transition()) {
    return NotAnimating;
  }

  auto startProp = oldProp;
  auto endProp = newProp;

  if (!startProp.has_value()) {
    startProp += DefaultValue;
  }

  if (!endProp.has_value()) {
    endProp += DefaultValue;
  }

  if (startProp == endProp) {
    return NotAnimating;
  }

  if (!(startProp.has_value() && endProp.has_value())) {
#ifndef NDEBUG
    __debugbreak();
#endif
    return NotAnimating;
  }

  const TValue startValue = startProp.value();
  const TValue endValue = endProp.value();

  if (startValue == endValue) {
    return NotAnimating;
  }

  if constexpr (std::floating_point<TValue>) {
    if (std::isnan(startValue) || std::isnan(endValue)) {
      return NotAnimating;
    }
    if (almost_equal(startValue, endValue)) {
      return NotAnimating;
    }
  }

  const auto& transition = newProp.transition();
  const auto duration
    = newProp.transition().mDuration * (SlowAnimations ? 10 : 1);

  auto& transitionState = this->*TStateProperty;
  if (
    (!transitionState.has_value()) || transitionState->mEndValue != endValue) {
    transitionState = {
      .mStartValue = startValue,
      .mStartTime = now + transition.mDelay,
      .mEndValue = endValue,
      .mEndTime = now + transition.mDelay + duration,
    };
    newProp = startValue;
    return Animating;
  }

  if (transitionState->mEndTime < now) {
    transitionState.reset();
    return NotAnimating;
  }

  newProp = transitionState->Evaluate(transition, now);

  if (newProp == transitionState->mEndValue) {
    transitionState.reset();
    return NotAnimating;
  }
  if constexpr (std::floating_point<TValue>) {
    if (newProp.has_value() && almost_equal(newProp.value(), endValue)) {
      newProp = endValue;
      transitionState.reset();
      return NotAnimating;
    }
  }

  transitionState = {
    .mStartValue = newProp.value(),
    .mStartTime = std::max(transitionState->mStartTime, now),
    .mEndValue = endValue,
    .mEndTime = std::max(transitionState->mEndTime, now + duration),
  };

  return Animating;
}

Widget::StyleTransitions::ApplyResult Widget::StyleTransitions::Apply(
  const Style& oldStyle,
  Style* newStyle) {
  const DebugCopy<Style> targetStyle {*newStyle};
  const DebugCopy<StyleTransitions> originalState {*this};

  using enum ApplyResult;
  if (!SystemSettings::Get().GetAnimationsEnabled()) {
    return NotAnimating;
  }
  const auto now = std::chrono::steady_clock::now();

  auto ret = NotAnimating;

#define APPLY_TRANSITION(X) \
  if ( \
    Apply<&Style::m##X, &StyleTransitions::m##X>(now, oldStyle, newStyle) \
    == Animating) { \
    ret = Animating; \
  }
  FUI_STYLE_PROPERTIES(APPLY_TRANSITION)
#undef APPLY_TRANSITION

  const auto CheckTransition =
    [&]<auto proj, auto stateProj>(const auto& name) {
      const auto& targetValue = std::invoke(proj, targetStyle);
      auto& newValue = std::invoke(proj, newStyle);
      const auto& oldState = std::invoke(stateProj, originalState);
      auto& newState = std::invoke(stateProj, this);
      if (targetValue.has_value() && targetValue.value() != newValue) {
        std::println(
          stderr,
          "Animation result mismatch on {} - value changed, but not animating",
          name);
        if (IsDebuggerPresent()) {
          __debugbreak();
          // Call it again so we can step through :)
          newValue = targetValue;
          newState = oldState;
          (void)Apply<proj, stateProj>(now, oldStyle, newStyle);
        }
      }
    };
  if constexpr (DebugAnimations) {
    if (ret == NotAnimating) {
#define CHECK_TRANSITION(X) \
  CheckTransition.template operator()<&Style::m##X, &StyleTransitions::m##X>( \
    #X);
      FUI_STYLE_PROPERTIES(CHECK_TRANSITION);
#undef CHECK_TRANSITION
    }
  }
  return ret;
}

}// namespace FredEmmott::GUI::Widgets