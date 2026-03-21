// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "SelectionPill.hpp"

#include "FredEmmott/GUI/Renderer.hpp"
#include "FredEmmott/GUI/StaticTheme/Common.hpp"

namespace FredEmmott::GUI::detail {
namespace {
constexpr std::chrono::milliseconds AnimationDuration {167};
constexpr auto AnimationEasing = EasingFunctions::CubicBezier(
  StaticTheme::Common::ControlFastOutSlowInKeySpline);

}// namespace
void SelectionPill::Transition(const State state) {
  mState = state;
  mStartAnimationAt = std::chrono::steady_clock::now();
  using enum State;
  if (
    state == GainingSelectionFromAbove || state == GainingSelectionFromBelow) {
    mStartAnimationAt += AnimationDuration;
  }
}

void SelectionPill::Tick(const std::chrono::steady_clock::time_point now) {
  using enum State;
  if (mState == Selected || mState == NotSelected) {
    return;
  }
  mNow = now;
  if ((now - mStartAnimationAt) >= AnimationDuration) {
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
        std::unreachable();
    }
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
      {x, rect.GetBottom() - offset},
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

  switch (mState) {
    case Selected:
    case NotSelected:
      std::unreachable();
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

  const auto t = (mNow - mStartAnimationAt)
    / std::chrono::duration_cast<std::chrono::duration<float>>(
                   AnimationDuration);
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
}// namespace FredEmmott::GUI::detail