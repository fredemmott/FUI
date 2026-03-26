// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "SelectionPill.hpp"

#include "FredEmmott/GUI/Renderer.hpp"
#include "FredEmmott/GUI/StaticTheme/Common.hpp"

namespace FredEmmott::GUI::detail {
namespace {
constexpr auto AnimationEasing = EasingFunctions::CubicBezier(
  StaticTheme::Common::ControlFastOutSlowInKeySpline);
}// namespace
void SelectionPill::Transition(const State state) {
  const auto now = std::chrono::steady_clock::now();

  using enum State;
  switch (state) {
    case Selected:
    case NotSelected:
      // no animation
      break;
    case LosingSelectionToAbove:
    case LosingSelectionToBelow:
      mStartAnimationAt = now;
      mFinishAnimationAt
        = now + StaticTheme::Common::ControlFastAnimationDuration;
      FUI_ASSERT(this->GetFrameRateRequirement().RequiresSmoothAnimation());
      break;
    case GainingSelectionFromAbove:
    case GainingSelectionFromBelow:
      // Wait for 'losing selection' animation
      mStartAnimationAt
        = now + StaticTheme::Common::ControlFastAnimationDuration;
      mFinishAnimationAt
        = mStartAnimationAt + StaticTheme::Common::ControlFastAnimationDuration;
      break;
    case SelectedPressed:
    case SelectedReleased:
      mStartAnimationAt = now;
      mFinishAnimationAt = now + mSelectedPressedAnimation.mDuration;
      FUI_ASSERT(this->GetFrameRateRequirement().RequiresSmoothAnimation());
      break;
  }
  mState = state;
}

void SelectionPill::Tick(const std::chrono::steady_clock::time_point now) {
  mNow = now;
  if (now < mFinishAnimationAt) {
    return;
  }
  using enum State;
  switch (mState) {
    case GainingSelectionFromAbove:
    case GainingSelectionFromBelow:
      mState = Selected;
      return;
    case LosingSelectionToAbove:
    case LosingSelectionToBelow:
      mState = NotSelected;
      return;
    case Selected:
    case NotSelected:
    case SelectedPressed:
    case SelectedReleased:
      break;
  }
}

void SelectionPill::Paint(
  Renderer* renderer,
  const Rect& rect,
  const Brush& brush,
  const float nominalHeight) const {
  using enum State;
  if (mState == NotSelected) {
    return;
  }

  const auto containerHeight = rect.GetHeight();
  const auto thickness = rect.GetWidth();

  const auto height = nominalHeight - thickness;
  const auto offset = (containerHeight - height) / 2;
  const auto x = rect.GetWidth() / 2;

  if (mState == Selected) {
    renderer->DrawLine(
      brush,
      {x, rect.GetTop() + offset},
      {x, rect.GetTop() + offset + height},
      thickness,
      StrokeCap::Round);
    return;
  }

  if (mNow < mStartAnimationAt) {
    FUI_ASSERT(
      mState == GainingSelectionFromAbove
      || mState == GainingSelectionFromBelow);
    return;
  }

  struct {
    float mInitialTop {};
    float mFinalTop {};
    float mInitialBottom {};
    float mFinalBottom {};
  } params;

  const auto pressedHeight
    = (nominalHeight * mSelectedPressedAnimation.mScale) - thickness;
  const auto pressedOffset = (containerHeight - pressedHeight) / 2;

  switch (mState) {
    case NotSelected:
    case Selected:
      std::unreachable();
    case SelectedPressed:
      params = {
        offset,
        pressedOffset,
        offset + height,
        pressedOffset + pressedHeight,
      };
      break;
    case SelectedReleased:
      params = {
        pressedOffset,
        offset,
        pressedOffset + pressedHeight,
        offset + height,
      };
      break;
    case GainingSelectionFromAbove:
      params = {
        offset,
        offset,
        -offset,
        offset + height,
      };
      break;
    case GainingSelectionFromBelow:
      params = {
        containerHeight + offset,
        offset,
        offset + height,
        offset + height,
      };
      break;
    case LosingSelectionToAbove:
      params = {
        offset,
        offset,
        offset + height,
        -offset,
      };
      break;
    case LosingSelectionToBelow:
      params = {
        offset,
        containerHeight + offset,
        offset + height,
        offset + height,
      };
      break;
  }

  if (mNow >= mFinishAnimationAt) {
    renderer->DrawLine(
      brush,
      {x, rect.GetTop() + params.mFinalTop},
      {x, rect.GetTop() + params.mFinalBottom},
      thickness,
      StrokeCap::Round);
    return;
  }

  const auto t = std::chrono::duration<float>(mNow - mStartAnimationAt)
    / (mFinishAnimationAt - mStartAnimationAt);
  FUI_ASSERT(t >= 0 && t <= 1);
  const auto eased = AnimationEasing(t);
  const auto top
    = Interpolation::Linear(params.mInitialTop, params.mFinalTop, eased);
  const auto bottom
    = Interpolation::Linear(params.mInitialBottom, params.mFinalBottom, eased);
  renderer->DrawLine(
    brush,
    {x, rect.GetTop() + top},
    {x, rect.GetTop() + bottom},
    thickness,
    StrokeCap::Round);
}

FrameRateRequirement SelectionPill::GetFrameRateRequirement() const noexcept {
  return FrameRateRequirement::SmoothAnimation {};
  const auto now = std::chrono::steady_clock::now();
  if (now >= mFinishAnimationAt) {
    return {};
  }
  if (now < mStartAnimationAt) {
    return FrameRateRequirement::After {mStartAnimationAt};
  }

  return FrameRateRequirement::SmoothAnimation {};
}
}// namespace FredEmmott::GUI::detail