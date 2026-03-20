// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "NavigationViewSettingsItem.hpp"

namespace FredEmmott::GUI::Widgets {

NavigationViewSettingsItem::NavigationViewSettingsItem(const id_type id)
  : NavigationViewItem(id) {
  this->SetIcon("\ue713");
}

NavigationViewSettingsItem::~NavigationViewSettingsItem() {}

void NavigationViewSettingsItem::Tick(
  const std::chrono::steady_clock::time_point& now) {
  using enum AnimationState;
  if (mAnimationState == Resting) {
    FUI_ASSERT(mNextAnimationState == Resting);
    return;
  }

  struct Params {
    float mStartValue {};
    float mEndValue {};
    std::chrono::milliseconds mDuration {};
    std::array<float, 4> mBezierPoints {};
  };
  const auto params = [](const AnimationState state) constexpr -> Params {
    // These are based on the generated C++ in WinUI3, which in turn is based
    // on some Lottie file that isn't included in the WinUI3 source tree
    switch (state) {
      case Resting:
        FUI_FATAL("'Resting' state should be unreachable");
      case OnPress:
        return {
          0,
          -20,
          std::chrono::milliseconds {125},
          {0.167, 0.167, 1, 1},
        };
      case OnRelease:
        return {
          -20,
          360,
          std::chrono::milliseconds {483},
          {0, 0, 0, 1},
        };
    }
    std::unreachable();
  }(mAnimationState);
  const auto elapsed = now - mAnimationStartTime;
  if (elapsed > params.mDuration) {
    this->SetIconRotation(params.mEndValue);
    mAnimationState = mNextAnimationState;
    mNextAnimationState = Resting;
    Tick(now);
    return;
  }

  const auto t
    = std::chrono::duration_cast<std::chrono::duration<float>>(elapsed)
    / params.mDuration;
  const auto ease = EasingFunctions::CubicBezier(params.mBezierPoints);
  const auto degrees
    = params.mStartValue + (ease(t) * (params.mEndValue - params.mStartValue));
  this->SetIconRotation(degrees);
}

FrameRateRequirement NavigationViewSettingsItem::GetFrameRateRequirement()
  const noexcept {
  if (mAnimationState != AnimationState::Resting) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return NavigationViewItem::GetFrameRateRequirement();
}

Widget::EventHandlerResult NavigationViewSettingsItem::OnMouseButtonPress(
  const MouseEvent& e) {
  if (
    e.Get<MouseEvent::ButtonPressEvent>().mPressedButtons
    == MouseButton::Left) {
    if (mAnimationState == AnimationState::Resting) {
      mAnimationState = AnimationState::OnPress;
      mAnimationStartTime = std::chrono::steady_clock::now();
    } else {
      mNextAnimationState = AnimationState::OnPress;
    }
  }

  return NavigationViewItem::OnMouseButtonPress(e);
}

Widget::EventHandlerResult NavigationViewSettingsItem::OnMouseButtonRelease(
  const MouseEvent& e) {
  if (
    e.Get<MouseEvent::ButtonReleaseEvent>().mReleasedButtons
    == MouseButton::Left) {
    if (mAnimationState == AnimationState::Resting) {
      mAnimationState = AnimationState::OnRelease;
      mAnimationStartTime = std::chrono::steady_clock::now();
    } else {
      mNextAnimationState = AnimationState::OnRelease;
    }
  }
  return NavigationViewItem::OnMouseButtonRelease(e);
}

}// namespace FredEmmott::GUI::Widgets