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
auto& as_ref(T&& value) {
  if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
    return *value;
  } else if constexpr (std::is_reference_v<T>) {
    return value;
  }
}

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

template <class T, class U>
  requires std::same_as<std::decay_t<T>, std::decay_t<U>>
  && (!requires(T t) { t.value(); })
constexpr bool almost_equal(T&& a, U&& b) {
  if (a == b) {
    return true;
  }
  if constexpr (std::floating_point<std::decay_t<T>>) {
    // YGUndefined is quiet NaN
    if (std::isnan(a) && std::isnan(b)) {
      return true;
    }
    // 0.1% is close enough for visual animations - we don't need max
    // float precision. If this ends up true for some property, specialize
    // like we do for defaults.
    return utility::almost_equal(a, b, 0.001f);
  }
  return false;
}
template <class T, class U>
  requires requires(T t, U u) {
    almost_equal(t.value(), u.value());
    { almost_equal(t.value(), u.value()) } -> std::same_as<bool>;
  }
constexpr bool almost_equal(T&& a, U&& b) {
  return almost_equal(std::forward<T>(a).value(), std::forward<U>(b).value());
}

}// namespace

template <class TValue>
Widget::StyleTransitions::ApplyResult Widget::StyleTransitions::Apply(
  const std::chrono::steady_clock::time_point now,
  const Style& oldStyle,
  Style* newStyle,
  const StylePropertyKey key) {
  using enum ApplyResult;
  constexpr auto DefaultValue = transition_default_value_v<TValue>;

  if (!oldStyle.mStorage.contains(key)) {
    mTransitions.erase(key);
    return NotAnimating;
  }

  if (!newStyle->mStorage.contains(key)) {
    mTransitions.erase(key);
    return NotAnimating;
  }

  const auto& oldProp = get<StyleProperty<TValue>>(oldStyle.mStorage[key]);
  auto& newProp = get<StyleProperty<TValue>>(newStyle->mStorage[key]);

  ///////////////////////////////////////////////////////
  //  1. Do we have a start, an end, and an animation? //
  ///////////////////////////////////////////////////////
  if (!(oldProp || newProp)) {
    mTransitions.erase(key);
    return NotAnimating;
  }

  if (!newProp.has_transition()) {
    mTransitions.erase(key);
    return NotAnimating;
  }

  // Optimization only, this case should be handled correctly by the code below
  if (
    newProp.transition().mDuration.count() == 0
    && newProp.transition().mDelay.count() == 0) {
    mTransitions.erase(key);
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
    mTransitions.erase(key);
    return NotAnimating;
  }

  /////////////////////////////////////////////////////
  //  2. Are the values different enough to animate? //
  /////////////////////////////////////////////////////

  const TValue startValue = startProp.value();
  const TValue endValue = endProp.value();

  if (almost_equal(startValue, endValue)) {
    newProp = endValue;
    mTransitions.erase(key);
    return NotAnimating;
  }

  /////////////////////////////////////
  //  3. Do we need a new animation? //
  /////////////////////////////////////

  const auto& transition = newProp.transition();
  const auto duration
    = newProp.transition().mDuration * (SlowAnimations ? 10 : 1);

  if (!mTransitions.contains(key)) {
    TransitionState<TValue> state {
      .mStartValue = startValue,
      .mStartTime = now + transition.mDelay,
      .mEndValue = endValue,
      .mEndTime = now + transition.mDelay + duration,
    };
    newProp = state.Evaluate(transition, now);
    if (almost_equal(newProp.value(), endValue)) {
      return NotAnimating;
    }
    mTransitions.emplace(key, std::move(state));
    return Animating;
  }

  auto& transitionState = get<TransitionState<TValue>>(mTransitions.at(key));

  // Target value has changed, so we need to update the animation
  if (!almost_equal(transitionState.mEndValue, endValue)) {
    const auto delay = (now >= transitionState.mStartTime)
      ? StyleTransition::Duration {}
      : (transition.mDelay - (transitionState.mStartTime - now));
    transitionState = {
      .mStartValue = oldProp.value(),
      .mStartTime = now + delay,
      .mEndValue = endValue,
      .mEndTime = std::max(transitionState.mEndTime, now + delay + duration),
    };
    newProp = oldProp.value();

    return Animating;
  }

  ///////////////////////////////////
  //  4. Use the current animation //
  ///////////////////////////////////

  newProp = transitionState.Evaluate(transition, now);

  // Has it finished?
  if (almost_equal(newProp.value(), endValue)) {
    newProp = endValue;
    mTransitions.erase(key);
    return NotAnimating;
  }

  // Nope, current animation is still progress
  FUI_ASSERT(
    transitionState.mEndTime > now,
    "Animations should end at their target value");
  return Animating;
}

Widget::StyleTransitions::ApplyResult Widget::StyleTransitions::Apply(
  const Style& oldStyle,
  Style* newStyle) {
  using enum ApplyResult;
  if (!SystemSettings::Get().GetAnimationsEnabled()) {
    return NotAnimating;
  }
  const auto now = std::chrono::steady_clock::now();

  auto ret = NotAnimating;

  for (auto&& [key, value]: newStyle->mStorage) {
    style_detail::VisitStyleProperty(
      key,
      [&]<class T>(const StyleProperty<T>&) {
        if constexpr (StyleProperty<T>::SupportsTransitions) {
          if (Apply<T>(now, oldStyle, newStyle, key) == Animating) {
            ret = Animating;
          }
        }
      },
      value);
  }
  return ret;
}// namespace FredEmmott::GUI::Widgets

}// namespace FredEmmott::GUI::Widgets