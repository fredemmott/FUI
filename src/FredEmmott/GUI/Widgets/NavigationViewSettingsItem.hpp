// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "NavigationViewItem.hpp"

namespace FredEmmott::GUI::Widgets {

class NavigationViewSettingsItem final : public NavigationViewItem {
 public:
  explicit NavigationViewSettingsItem(id_type id);
  ~NavigationViewSettingsItem() override;

 protected:
  void Tick(const std::chrono::steady_clock::time_point& now) override;
  FrameRateRequirement GetFrameRateRequirement() const noexcept override;
  EventHandlerResult OnMouseButtonPress(const MouseEvent&) override;
  EventHandlerResult OnMouseButtonRelease(const MouseEvent&) override;

 private:
  enum class AnimationState {
    Resting,
    OnPress,
    OnRelease,
  };
  AnimationState mAnimationState {AnimationState::Resting};
  AnimationState mNextAnimationState {AnimationState::Resting};
  std::chrono::steady_clock::time_point mAnimationStartTime {};
};

}// namespace FredEmmott::GUI::Widgets