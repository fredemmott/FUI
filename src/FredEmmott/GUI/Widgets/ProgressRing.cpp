// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// Portions derived from Microsoft UI XAML. `PaintOwnContent()` is a
// representation of the artwork in the WinUI3 control.

#include "ProgressRing.hpp"

#include "FredEmmott/GUI/StaticTheme/ProgressRing.hpp"
#include "FredEmmott/utility/almost_equal.hpp"

namespace FredEmmott::GUI::Widgets {
namespace {
constexpr LiteralStyleClass ProgressRingStyleClass {"ProgressRing"};
auto& ProgressRingStyle() {
  static const ImmutableStyle ret {
    Style().AspectRatio(1).MinHeight(16).MinWidth(16).Height(32).MaxHeight(32)};
  return ret;
}
}// namespace

ProgressRing::ProgressRing(Window* const window, const Kind kind)
  : Widget(window, ProgressRingStyleClass, ProgressRingStyle()),
    mKind(kind) {}

void ProgressRing::SetRange(const float minimum, const float maximum) {
  if (
    utility::almost_equal(minimum, mMinimum)
    && utility::almost_equal(maximum, mMaximum) && mKind == Kind::Determinate) {
    return;
  }
  mMinimum = minimum;
  mMaximum = maximum;
  mValue = std::clamp(mValue, minimum, maximum);
}

void ProgressRing::SetValue(const float value) {
  if (utility::almost_equal(value, mValue) && mKind == Kind::Determinate) {
    return;
  }
  if (value < mMinimum || value > mMaximum) [[unlikely]] {
    throw std::out_of_range("Value must be between minimum and maximum");
  }
  mValue = value;
}

void ProgressRing::SetIsActive(const bool value) {
  mIsActive = value;
}

void ProgressRing::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style&) const {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ProgressRing;
  if (mKind == Kind::Indeterminate && !mIsActive) {
    return;
  }
  const auto strokeRect = rect.WithInset(
    ProgressRingStrokeThickness / 2, ProgressRingStrokeThickness / 2);

  const auto theme = StaticTheme::GetCurrent();

  if (mKind == Kind::Determinate) {
    renderer->StrokeEllipse(
      ControlStrongStrokeColorDefault.Resolve(theme),
      strokeRect,
      ProgressRingStrokeThickness);
    // TODO (?): WinUI3 animates large changes here
    const auto sweepAngle = (mValue - mMinimum) * 360.f / (mMaximum - mMinimum);
    renderer->StrokeArc(
      ProgressRingForegroundThemeBrush.Resolve(theme),
      strokeRect,
      // E -> N
      270.0f,
      sweepAngle,
      ProgressRingStrokeThickness,
      StrokeCap::Round);
    return;
  }

  FUI_ASSERT(mKind == Kind::Indeterminate);

  static constexpr auto MaxTicks = 2000;
  const auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                       .count()
    % MaxTicks;
  const auto t = ticks / static_cast<float>(MaxTicks);
  FUI_ASSERT(t >= 0.0f && t <= 1.0f);

  static constexpr auto Ease
    = EasingFunctions::CubicBezier {0.167f, 0.167f, 0.833f, 0.833f};

  float startT = 0.0f;
  float endT = 0.0f;

  if (t < 0.5f) {
    const auto localT = t * 2.0f;
    endT = Ease(localT) * 0.5f;
  } else {
    const auto localT = (t - 0.5f) * 2.0f;
    startT = Ease(localT) * 0.5f;
    endT = 0.5f;
  }

  // Yep, not an integer number of rotations, but things line up
  const auto baseRotation = Ease(t) * 900.0f;
  const float startAngle = 270.0f + baseRotation + (startT * 360.0f);
  const float sweepAngle = (endT - startT) * 360.0f;
  renderer->StrokeArc(
    ProgressRingForegroundThemeBrush.Resolve(theme),
    strokeRect,
    startAngle,
    sweepAngle,
    ProgressRingStrokeThickness,
    StrokeCap::Round);
}

FrameRateRequirement ProgressRing::GetFrameRateRequirement() const noexcept {
  if (mKind == Kind::Indeterminate && mIsActive) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return Widget::GetFrameRateRequirement();
}

ProgressRing::~ProgressRing() = default;

}// namespace FredEmmott::GUI::Widgets