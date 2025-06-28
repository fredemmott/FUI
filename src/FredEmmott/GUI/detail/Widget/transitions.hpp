// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/widget_detail.hpp>

namespace FredEmmott::GUI::Widgets::widget_detail {

template <class T>
struct TransitionState {
  using option_type = TransitionState;
};

template <class T>
  requires StyleProperty<T>::SupportsTransitions
struct TransitionState<T> {
  using option_type = std::optional<TransitionState>;
  using time_point = std::chrono::steady_clock::time_point;

  T mStartValue;
  time_point mStartTime;
  T mEndValue;
  time_point mEndTime;

  [[nodiscard]] T Evaluate(const auto& transition, const time_point& now)
    const noexcept {
    if (now < mStartTime) {
      return mStartValue;
    }
    if (now > mEndTime) {
      return mEndValue;
    }
    const auto duration = mEndTime - mStartTime;
    const auto elapsed = now - mStartTime;
    const auto t = static_cast<double>(elapsed.count()) / duration.count();
    const auto eased = transition.mEasingFunction(t);
    return Interpolation::Linear(mStartValue, mEndValue, eased);
  }
};

}// namespace FredEmmott::GUI::Widgets::widget_detail

namespace FredEmmott::GUI::Widgets {

struct Widget::StyleTransitions {
  void Apply(const Style& oldStyle, Style* newStyle);

 private:
  template <auto TStyleProperty>
  static constexpr bool supports_transitions_v = std::decay_t<
    decltype(std::declval<Style>().*TStyleProperty)>::SupportsTransitions;

  template <auto TStyleProperty, auto TStateProperty>
  void Apply(
    std::chrono::steady_clock::time_point now,
    const Style& oldStyle,
    Style* newStyle)
    requires(supports_transitions_v<TStyleProperty>);

  template <auto TStyleProperty, auto TStateProperty>
  void Apply(
    [[maybe_unused]] std::chrono::steady_clock::time_point now,
    [[maybe_unused]] const Style& old,
    [[maybe_unused]] Style* newStyle)
    requires(!supports_transitions_v<TStyleProperty>)
  {
  }

#define DECLARE_TRANSITION_DATA(X) \
  FUI_NO_UNIQUE_ADDRESS \
  widget_detail::TransitionState< \
    decltype(Style::m##X)::value_type>::option_type m##X;
  FUI_STYLE_PROPERTIES(DECLARE_TRANSITION_DATA)
#undef TRANSITION_DATA
};
}// namespace FredEmmott::GUI::Widgets
