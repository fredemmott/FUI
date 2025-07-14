// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/widget_detail.hpp>

namespace FredEmmott::GUI::Widgets::widget_detail {

template <class T>
  requires StyleProperty<T>::SupportsTransitions
struct TransitionState {
  using time_point = std::chrono::steady_clock::time_point;

  T mStartValue;
  time_point mStartTime;
  T mEndValue;
  time_point mEndTime;

  [[nodiscard]] T Evaluate(const auto& transition, const time_point& now)
    const noexcept {
    if (now >= mEndTime) {
      return mEndValue;
    }
    if (now < mStartTime) {
      return mStartValue;
    }
    const auto duration = mEndTime - mStartTime;
    const auto elapsed = now - mStartTime;
    const auto t = static_cast<double>(elapsed.count()) / duration.count();
    const auto eased = transition.mEasingFunction(t);
    return Interpolation::Linear(mStartValue, mEndValue, eased);
  }
};

template <class T>
struct TransitionStorage {
  using type = std::monostate;
};

template <class T>
  requires StyleProperty<T>::SupportsTransitions
struct TransitionStorage<T> {
  using type
    = std::unordered_map<style_detail::StylePropertyKey, TransitionState<T>>;
};
template <class t>
using TransitionStorage_t = typename TransitionStorage<t>::type;

}// namespace FredEmmott::GUI::Widgets::widget_detail

namespace FredEmmott::GUI::Widgets {

struct Widget::StyleTransitions {
  enum class ApplyResult {
    NotAnimating,
    Animating,
  };
  [[nodiscard]]
  ApplyResult Apply(const Style& oldStyle, Style* newStyle);

 private:
  template <class TValue>
  static constexpr bool supports_transitions_v
    = StyleProperty<TValue>::SupportsTransitions;

  template <class TValue>
  [[nodiscard]]
  ApplyResult Apply(
    std::chrono::steady_clock::time_point now,
    const Style& oldStyle,
    Style* newStyle,
    widget_detail::TransitionStorage_t<TValue>& transitions,
    style_detail::StylePropertyKey);

  template <class TValue>
  [[nodiscard]]
  ApplyResult Apply(
    std::chrono::steady_clock::time_point now,
    const Style& oldStyle,
    Style* newStyle,
    widget_detail::TransitionStorage_t<TValue>& transitions)
    requires(supports_transitions_v<TValue>);

  template <class TValue>
  [[nodiscard]]
  ApplyResult Apply(
    [[maybe_unused]] std::chrono::steady_clock::time_point now,
    [[maybe_unused]] const Style& oldStyle,
    [[maybe_unused]] Style* newStyle,
    [[maybe_unused]] widget_detail::TransitionStorage_t<TValue>& transitions)
    requires(!supports_transitions_v<TValue>)
  {
    return ApplyResult::NotAnimating;
  }

#define DECLARE_TRANSITION_DATA(TYPE, NAME) \
  FUI_NO_UNIQUE_ADDRESS widget_detail::TransitionStorage_t<TYPE> \
    m##NAME##Transitions;
  FUI_ENUM_STYLE_PROPERTY_TYPES(DECLARE_TRANSITION_DATA)
#undef DECLARE_TRANSITION_DATA
};
}// namespace FredEmmott::GUI::Widgets
