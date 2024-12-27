// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/detail/Widget/transitions.hpp>
#include <FredEmmott/GUI/detail/widget_detail.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {
using namespace widget_detail;

namespace {
template <class T>
struct transition_default_value_t : constant_t<std::nullopt> {};
template <>
struct transition_default_value_t<SkScalar> : constant_t<0> {};
template <>
struct transition_default_value_t<Brush> : constant_t<SK_ColorTRANSPARENT> {};

template <class T>
constexpr auto transition_default_value_v
  = transition_default_value_t<T>::value;
}// namespace

void Widget::StyleTransitions::Apply(
  std::chrono::steady_clock::time_point now,
  const Style& oldStyle,
  Style* newStyle,
  auto Style::* styleProperty,
  auto StyleTransitions::* stateProperty) {
  using TValue =
    typename std::decay_t<decltype(oldStyle.*styleProperty)>::value_type;
  constexpr auto DefaultValue = transition_default_value_v<TValue>;

  const auto oldOpt = (oldStyle.*styleProperty);
  auto& newOpt = (newStyle->*styleProperty);

  if (!newOpt.has_transition()) {
    return;
  }

  auto startOpt = oldOpt;
  auto endOpt = newOpt;

  if (!startOpt.has_value()) {
    startOpt += DefaultValue;
  }

  if (!endOpt.has_value()) {
    endOpt += DefaultValue;
  }

  if (startOpt == endOpt) {
    return;
  }

  if (!(startOpt.has_value() && endOpt.has_value())) {
#ifndef NDEBUG
    __debugbreak();
#endif
    return;
  }

  const TValue startValue = startOpt.value();
  const TValue endValue = endOpt.value();
  if constexpr (std::floating_point<TValue>) {
    if (std::isnan(startValue) || std::isnan(endValue)) {
      return;
    }
  }

  constexpr bool DebugAnimations = false;
  const auto duration
    = newOpt.transition().mDuration * (DebugAnimations ? 10 : 1);

  auto& transitionState = this->*stateProperty;
  if (!transitionState.has_value()) {
    transitionState = {
      .mStartValue = startValue,
      .mStartTime = now,
      .mEndValue = endValue,
      .mEndTime = now + duration,
    };
    newOpt = startValue;
    return;
  }

  if (transitionState->mEndTime < now) {
    transitionState.reset();
    return;
  }

  newOpt = transitionState->Evaluate(newOpt.transition(), now);

  if (transitionState->mEndValue == endValue) {
    return;
  }
  transitionState = {
    .mStartValue = newOpt.value(),
    .mStartTime = now,
    .mEndValue = endValue,
    .mEndTime = now + duration,
  };
}

void Widget::ApplyStyleTransitions(Style* newStyle) {
  const auto now = std::chrono::steady_clock::now();

  const auto applyIfHasTransition = [this, now, newStyle](
                                      auto styleP, auto stateP) {
    using T = std::decay_t<decltype(newStyle->*styleP)>;
    if constexpr (requires(T prop) { prop.transition(); }) {
      mStyleTransitions->Apply(now, mComputedStyle, newStyle, styleP, stateP);
    }
  };

#define APPLY_TRANSITION(X) \
  { \
    const auto propName = #X; \
    applyIfHasTransition(&Style::m##X, &StyleTransitions::m##X); \
  }
  FUI_STYLE_PROPERTIES(APPLY_TRANSITION)
#undef APPLY_TRANSITION
}

}// namespace FredEmmott::GUI::Widgets