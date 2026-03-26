// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <FredEmmott/GUI/FrameRateRequirement.hpp>
#include <FredEmmott/GUI/Rect.hpp>
#include <FredEmmott/GUI/StaticTheme.hpp>
#include <chrono>

namespace FredEmmott::GUI {
class Brush;
class Renderer;
}// namespace FredEmmott::GUI

namespace FredEmmott::GUI::detail {

class SelectionPill {
 public:
  enum class State {
    NotSelected,
    Selected,// e.g. NavigationViewItem
    SelectedPressed,// e.g. ComboBoxItem
    SelectedReleased,// ditto
    GainingSelectionFromAbove,
    GainingSelectionFromBelow,
    LosingSelectionToAbove,
    LosingSelectionToBelow,
  };

  SelectionPill() = delete;
  explicit constexpr SelectionPill(const State state) : mState(state) {}

  void Transition(State state);

  void Tick(std::chrono::steady_clock::time_point);
  void Paint(Renderer* renderer, const Rect&, const Brush&, float height) const;

  [[nodiscard]]
  FrameRateRequirement GetFrameRateRequirement() const noexcept;

  [[nodiscard]]
  constexpr State GetState() const noexcept {
    return mState;
  }

  void SetSelectedPressedAnimation(
    const float scale,
    const std::chrono::steady_clock::duration duration) {
    mSelectedPressedAnimation = {scale, duration};
  }

 private:
  struct SelectedPressedAnimation {
    float mScale {1.0f};
    std::chrono::steady_clock::duration mDuration {
      StaticTheme::Common::ControlFastAnimationDuration};
  };

  State mState {State::NotSelected};
  std::chrono::steady_clock::time_point mStartAnimationAt {};
  std::chrono::steady_clock::time_point mFinishAnimationAt {};
  std::chrono::steady_clock::time_point mNow {};

  SelectedPressedAnimation mSelectedPressedAnimation {};
};

}// namespace FredEmmott::GUI::detail