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

template <class T, class U>
  requires std::same_as<std::decay_t<T>, std::decay_t<U>>
constexpr bool almost_equal(T&& a, U&& b) {
  if (a == b) {
    return true;
  }
  if constexpr (std::floating_point<std::decay_t<T>>) {
    // 0.1% is close enough for visual animations - we don't need max
    // float precision. If this ends up true for some property, specialize
    // like we do for defaults.
    return utility::almost_equal(a, b, 0.001f);
  }
  return false;
}

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
  auto& transitionState = this->*TStateProperty;

  ///////////////////////////////////////////////////////
  //  1. Do we have a start, an end, and an animation? //
  ///////////////////////////////////////////////////////

  if (!newProp.has_transition()) {
    transitionState.reset();
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

  if (!(startProp.has_value() && endProp.has_value())) {
    if constexpr (DebugAnimations) {
      // Should be unreachable - we should always have a valid DefaultValue
      // for animatable properties
      __debugbreak();
    }
    transitionState.reset();
    return NotAnimating;
  }

  /////////////////////////////////////////////////////
  //  2. Are the values different enough to animate? //
  /////////////////////////////////////////////////////

  const TValue startValue = startProp.value();
  const TValue endValue = endProp.value();

  if (almost_equal(startValue, endValue)) {
    transitionState.reset();
    return NotAnimating;
  }

  /////////////////////////////////////
  //  3. Do we need a new animation? //
  /////////////////////////////////////

  const auto& transition = newProp.transition();
  const auto duration
    = newProp.transition().mDuration * (SlowAnimations ? 10 : 1);

  if (!transitionState.has_value()) {
    transitionState = {
      .mStartValue = startValue,
      .mStartTime = now + transition.mDelay,
      .mEndValue = endValue,
      .mEndTime = now + transition.mDelay + duration,
    };
    newProp = startValue;
    return Animating;
  }

  // Target value has changed, so we need to update the animation
  if (!almost_equal(transitionState->mEndValue, endValue)) {
    const auto delay = (now >= transitionState->mStartTime)
      ? StyleTransition::Duration {}
      : (transition.mDelay - (transitionState->mStartTime - now));
    transitionState = {
      .mStartValue = oldProp.value(),
      .mStartTime = now + delay,
      .mEndValue = endValue,
      .mEndTime = std::max(transitionState->mEndTime, now + delay + duration),
    };
    newProp = oldProp.value();

    return Animating;
  }

  ///////////////////////////////////
  //  4. Use the current animation //
  ///////////////////////////////////

  newProp = transitionState->Evaluate(transition, now);

  // Has it finished?
  if (almost_equal(newProp.value(), endValue)) {
    newProp = endValue;
    transitionState.reset();
    return NotAnimating;
  }

  // Nope, current animation is still progress
  FUI_ASSERT(
    transitionState->mEndTime > now,
    "Animations should end at their target value");
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

  const auto CheckTransition
    = [&]<auto proj, auto stateProj>(const auto& name) {
        if constexpr (DebugAnimations) {
          const auto& targetValue = std::invoke(proj, targetStyle);
          auto& newValue = std::invoke(proj, newStyle);
          const auto& oldState = std::invoke(stateProj, originalState);
          auto& newState = std::invoke(stateProj, this);
          if (targetValue.has_value() && targetValue.value() != newValue) {
            std::println(
              stderr,
              "Animation result mismatch on {} - value changed, but not "
              "animating",
              name);
            if (IsDebuggerPresent()) {
              __debugbreak();
              // Call it again so we can step through :)
              newValue = targetValue;
              newState = oldState;
              (void)Apply<proj, stateProj>(now, oldStyle, newStyle);
            }
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