// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/detail/Widget/transitions.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class ToggleSwitchKnob final : public Widget {
 public:
  explicit ToggleSwitchKnob(Window*);
  ~ToggleSwitchKnob() override;

 protected:
  [[nodiscard]]
  FrameRateRequirement GetFrameRateRequirement() const noexcept override;
  void Tick(const std::chrono::steady_clock::time_point& now) override;
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  enum class State {
    Disabled,
    Normal,
    Hovered,
    Active,
  };

  template <class T>
  using Animated = widget_detail::TransitionState<T>;

  Animated<float> mThumbHeight {};
  Animated<float> mThumbCenter {};
  Animated<float> mThumbWidth {};

  std::tuple<State, bool> mState {};
  void SetState(State);
};

}// namespace FredEmmott::GUI::Widgets
