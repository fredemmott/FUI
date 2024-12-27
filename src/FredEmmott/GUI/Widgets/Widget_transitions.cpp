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

void Widget::ApplyStyleTransitions(Style* newStyle) {
  const auto state = mStyleTransitions.get();
  const auto now = std::chrono::steady_clock::now();

  auto apply = [now, newStyle, oldStyle = &mComputedStyle, state](
                 auto styleP, auto stateP) {
    using TValue =
      typename std::decay_t<decltype(oldStyle->*styleP)>::value_type;
    constexpr auto DefaultValue = transition_default_value_v<TValue>;

    auto oldOpt = (oldStyle->*styleP);
    auto& newOpt = (newStyle->*styleP);
    auto targetOpt = newOpt;

    if (!oldOpt.has_value()) {
      oldOpt += DefaultValue;
    }

    if (!newOpt.has_value()) {
      targetOpt += DefaultValue;
    }

    if (oldOpt == newOpt) {
      return;
    }

    if (!(oldOpt.has_value() && targetOpt.has_value())) {
#ifndef NDEBUG
      __debugbreak();
#endif
      return;
    }
    const TValue oldValue = oldOpt.value();
    const TValue newValue = targetOpt.value();
    if constexpr (std::floating_point<TValue>) {
      if (std::isnan(oldValue) || std::isnan(newValue)) {
        return;
      }
    }

    constexpr bool DebugAnimations = false;
    const auto duration
      = newOpt.transition().mDuration * (DebugAnimations ? 10 : 1);

    auto& transitionState = state->*stateP;
    if (transitionState.has_value()) {
      if (transitionState->mEndTime < now) {
        transitionState.reset();
        return;
      }

      if (transitionState->mEndValue != newValue) {
        transitionState->mStartValue
          = transitionState->Evaluate(newOpt.transition(), now);
        transitionState->mStartTime = now;
        transitionState->mEndTime = now + duration,
        transitionState->mEndValue = newValue;
        newOpt = transitionState->mStartValue;
        return;
      }
      newOpt = transitionState->Evaluate(newOpt.transition(), now);
      return;
    }
    transitionState = {
      .mStartValue = oldValue,
      .mStartTime = now,
      .mEndValue = newValue,
      .mEndTime = now + duration,
    };
    newOpt = oldValue;
  };

  const auto applyIfHasTransition
    = [newStyle, apply](auto styleP, auto stateP) {
        auto& prop = newStyle->*styleP;
        if constexpr (requires { prop.has_transition(); }) {
          if (prop.has_transition()) {
            apply(styleP, stateP);
          }
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