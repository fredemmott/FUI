// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Widgets/Widget.hpp>
#include <FredEmmott/GUI/detail/widget_detail.hpp>

namespace FredEmmott::GUI::Widgets::widget_detail {

template <class T>
struct TransitionState;

template <class T>
  requires(!StyleProperty<T>::SupportsTransitions)
struct TransitionState<T> : std::monostate {};

template <class T>
  requires StyleProperty<T>::SupportsTransitions
struct TransitionState<T> {
  using value_type = T;
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
    style_detail::StylePropertyKey);

  utility::unordered_map<
    style_detail::StylePropertyKey,
    utility::drop_last_t<
      std::variant,
#define DECLARE_TRANSITION_DATA(TYPE, NAME) \
  widget_detail::TransitionState<TYPE>,
      FUI_ENUM_STYLE_PROPERTY_TYPES(DECLARE_TRANSITION_DATA)
#undef DECLARE_TRANSITION_DATA
        void>>
    mTransitions;
};
}// namespace FredEmmott::GUI::Widgets
