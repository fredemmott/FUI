// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <FredEmmott/GUI/Rect.hpp>
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
    Selected,
    GainingSelectionFromAbove,
    GainingSelectionFromBelow,
    LosingSelectionToAbove,
    LosingSelectionToBelow,
  };

  SelectionPill() = delete;
  constexpr SelectionPill(const State state) : mState(state) {}

  void Transition(const State state);

  void Tick(std::chrono::steady_clock::time_point);
  void Paint(Renderer* renderer, const Rect&, const Brush&, float height) const;

  [[nodiscard]]
  bool IsAnimating() const noexcept {
    return mState != State::NotSelected && mState != State::Selected;
  }

 private:
  State mState {State::NotSelected};
  std::chrono::steady_clock::time_point mStartAnimationAt {};
  std::chrono::steady_clock::time_point mNow {};
};

}// namespace FredEmmott::GUI::detail