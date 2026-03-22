// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/Slider.hpp>
#include <FredEmmott/GUI/events/KeyCode.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <algorithm>

#include "FredEmmott/GUI/FocusManager.hpp"
#include "FredEmmott/utility/almost_equal.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {

using namespace StaticTheme::Slider;
using namespace StaticTheme::Common;

static_assert(SliderHorizontalThumbWidth == SliderHorizontalThumbHeight);
static_assert(SliderVerticalThumbWidth == SliderVerticalThumbHeight);
static_assert(SliderHorizontalThumbWidth == SliderVerticalThumbWidth);
constexpr auto SliderThumbLength = SliderHorizontalThumbWidth;
constexpr auto SliderHalfThumbLength = SliderThumbLength / 2.0f;

constexpr auto SliderInnerThumbScaleAnimationDuration
  = ControlFastAnimationDuration;
constexpr auto SliderInnerThumbScaleAnimationEasingFunction
  = EasingFunctions::CubicBezier(ControlFastOutSlowInKeySpline);
constexpr auto SliderInnerThumbSize = 0.86f;
constexpr auto SliderInnerThumbScalePointerOver = 1.167f;
constexpr auto SliderInnerThumbScalePressed = 0.71f;
constexpr auto SliderInnerThumbScaleDisabled = 1.167f;

auto& SliderBaseStyle() {
  static const ImmutableStyle ret {Style().AlignItems(YGAlignCenter)};
  return ret;
}

auto& HorizontalSliderStyle() {
  static const ImmutableStyle ret {
    SliderBaseStyle()
    + Style()
        .FlexDirection(YGFlexDirectionRow)
        .Height(SliderHorizontalHeight)
        .MinHeight(SliderHorizontalHeight)};
  return ret;
}

auto& VerticalSliderStyle() {
  static const ImmutableStyle ret {
    SliderBaseStyle()
    + Style()
        .FlexDirection(YGFlexDirectionColumn)
        .Width(SliderVerticalWidth)
        .MinWidth(SliderVerticalWidth)};
  return ret;
}

struct SliderBrushes {
  Brush mTrackFill;
  Brush mTrackValueFill;
  Brush mTickBarFill;

  Brush mOuterThumbStroke;
  Brush mOuterThumbFill;
};

enum class SliderState : uint8_t {
  Disabled,
  Normal,
  Hover,
  Active,
};

Brush GetSliderInnerThumbBrush(const SliderState state) {
  // Thumb hover state is indepednent of slider pointer-hover
  switch (state) {
    case SliderState::Normal:
      return SliderThumbBackground;
    case SliderState::Hover:
      return SliderThumbBackgroundPointerOver;
    case SliderState::Active:
      return SliderThumbBackgroundPressed;
    case SliderState::Disabled:
      return SliderThumbBackgroundDisabled;
  }
  std::unreachable();
}

SliderBrushes GetSliderBrushes(const SliderState state) {
  // While WinUI3 XAML defines SliderThumbPressedBorderThemeBrush and friends,
  // they're unused by WinUI3. SliderThumbBorderBrush is used for all
  // states instead.
  switch (state) {
    case SliderState::Normal:
      return {
        .mTrackFill = SliderTrackFill,
        .mTrackValueFill = SliderTrackValueFill,
        .mTickBarFill = SliderTickBarFill,
        .mOuterThumbStroke = SliderThumbBorderBrush,
        .mOuterThumbFill = SliderOuterThumbBackground,
      };
    case SliderState::Hover:
      return {
        .mTrackFill = SliderTrackFillPointerOver,
        .mTrackValueFill = SliderTrackValueFillPointerOver,
        .mTickBarFill = SliderTickBarFill,
        .mOuterThumbStroke = SliderThumbBorderBrush,
        .mOuterThumbFill = SliderOuterThumbBackground,
      };
    case SliderState::Active:
      return {
        .mTrackFill = SliderTrackFillPressed,
        .mTrackValueFill = SliderTrackValueFillPressed,
        .mTickBarFill = SliderTickBarFill,
        .mOuterThumbStroke = SliderThumbBorderBrush,
        .mOuterThumbFill = SliderOuterThumbBackground,
      };
    case SliderState::Disabled:
      return {
        .mTrackFill = SliderTrackFillDisabled,
        .mTrackValueFill = SliderTrackValueFillDisabled,
        .mTickBarFill = SliderTickBarFill,
        .mOuterThumbStroke = SliderThumbBorderBrush,
        .mOuterThumbFill = SliderOuterThumbBackground,
      };
  };
  std::unreachable();
}

}// namespace

Slider::Slider(const id_type id, const Orientation orientation)
  : Widget(
      id,
      LiteralStyleClass("Slider"),
      ((orientation == Orientation::Horizontal) ? HorizontalSliderStyle()
                                                : VerticalSliderStyle())),
    IFocusable(this),
    mOrientation(orientation) {
  SetThumbState(std::to_underlying(SliderState::Normal));
}

void Slider::SetValue(const float value) {
  const auto clamped = std::clamp(value, mMinimum, mMaximum);
  if (utility::almost_equal(clamped, mValue)) {
    return;
  }
  mWasChanged = true;
  mValue = clamped;
}

float Slider::GetValue() const {
  return mValue;
}

void Slider::SetStepFrequency(const float frequency) {
  if (frequency < 0.0f) [[unlikely]] {
    throw std::logic_error("Step frequency must be positive");
  }
  if (frequency < std::numeric_limits<float>::epsilon()) {
    mStepFrequency = {};
  } else {
    mStepFrequency = frequency;
  }
}

void Slider::SetRange(const float min, const float max) {
  mMinimum = min;
  mMaximum = max;
  mValue = std::clamp(mValue, mMinimum, mMaximum);
}

float Slider::GetSnappedValue(float value) const noexcept {
  const auto snapFrequency
    = (mSnapTo == SnapTo::Steps) ? mStepFrequency : mTickFrequency;
  if (snapFrequency >= std::numeric_limits<float>::epsilon()) {
    const auto offset = value - mMinimum;
    value = mMinimum + (std::round(offset / snapFrequency) * snapFrequency);
  }
  return std::clamp(value, mMinimum, mMaximum);
}

Slider::Layout Slider::GetLayout(const float length) const noexcept {
  static constexpr auto thumbMin = SliderHalfThumbLength;
  const auto thumbMax = length - SliderHalfThumbLength;

  const auto thumbValue = mDraggingValue.value_or(mValue);
  const auto thumbRatio = (thumbValue - mMinimum) / (mMaximum - mMinimum);

  return Layout {
    .mTrackStart = 0,
    .mThumbMin = thumbMin,
    .mThumbMax = thumbMax,
    .mTrackEnd = length,
    .mThumbPoint = thumbMin + (thumbRatio * (thumbMax - thumbMin)),
  };
}

Widget::EventHandlerResult Slider::OnMouseButtonPress(const MouseEvent& event) {
  const auto parentResult = Widget::OnMouseButtonPress(event);
  if (this->IsDisabled() || !event.IsValid()) {
    return parentResult;
  }
  mDraggingValue = mValue;
  StartMouseCapture();
  return OnMouseMove(event);
}

Widget::EventHandlerResult Slider::OnMouseButtonRelease(const MouseEvent& e) {
  if (mDraggingValue) {
    EndMouseCapture();

    this->SetValue(this->GetSnappedValue(*mDraggingValue));
    mDraggingValue.reset();
  }
  std::ignore = Widget::OnMouseButtonRelease(e);
  this->OnMouseMove(e);
  if (const auto fm = FocusManager::Get()) {
    fm->GiveImplicitFocus(this);
  }
  return EventHandlerResult::StopPropagation;
}

float Slider::GetSnappedDraggingValue() const {
  return GetSnappedValue(*mDraggingValue);
}

Point Slider::GetTrackOriginOffset() const {
  const auto yoga = GetLayoutNode();
  if (mOrientation == Orientation::Horizontal) {
    const auto layout = GetLayout(YGNodeLayoutGetWidth(yoga));
    return Point {
      layout.mTrackStart,
      (YGNodeLayoutGetHeight(yoga) - *SliderTrackThemeHeight->Resolve()) / 2};
  }

  const auto layout = GetLayout(YGNodeLayoutGetHeight(yoga));
  return Point {
    (YGNodeLayoutGetWidth(yoga) - *SliderTrackThemeHeight->Resolve()) / 2,
    layout.mTrackStart,
  };
}

float Slider::GetThumbCenterOffsetWithinTrack() const {
  const auto yoga = GetLayoutNode();
  if (mOrientation == Orientation::Horizontal) {
    const auto layout = GetLayout(YGNodeLayoutGetWidth(yoga));
    return layout.mThumbPoint - layout.mTrackStart;
  }
  const auto layout = GetLayout(YGNodeLayoutGetHeight(yoga));
  return layout.mThumbPoint - layout.mTrackStart;
}

float Slider::GetTrackLength() const {
  const auto yoga = this->GetLayoutNode();
  if (mOrientation == Orientation::Horizontal) {
    const auto layout = GetLayout(YGNodeLayoutGetWidth(yoga));
    return layout.mTrackEnd - layout.mTrackStart;
  }
  const auto layout = GetLayout(YGNodeLayoutGetHeight(yoga));
  return layout.mTrackEnd - layout.mTrackStart;
}

Widget::EventHandlerResult Slider::OnMouseMove(const MouseEvent& event) {
  const auto isHorizontal = mOrientation == Orientation::Horizontal;
  const auto mouseOffset
    = isHorizontal ? event.GetPosition().mX : event.GetPosition().mY;

  const auto fullLength = isHorizontal
    ? YGNodeLayoutGetWidth(GetLayoutNode())
    : YGNodeLayoutGetHeight(GetLayoutNode());
  const auto layout = GetLayout(fullLength);
  const auto yoga = GetLayoutNode();
  const auto thumbCenter = isHorizontal ? Point {
    layout.mThumbPoint,
    YGNodeLayoutGetHeight(yoga) / 2,
  } : Point {
    YGNodeLayoutGetWidth(yoga) / 2,
    YGNodeLayoutGetHeight(yoga) - layout.mThumbPoint,
  };
  const auto thumb = Rect::FromCenterAndSize(
    thumbCenter, {SliderThumbLength, SliderThumbLength});

  if (thumb.ContainsPoint(event.GetPosition())) {
    if ((event.mButtons & MouseButton::Left) == MouseButton::Left) {
      SetThumbState(std::to_underlying(SliderState::Active));
    } else {
      SetThumbState(std::to_underlying(SliderState::Hover));
    }
  } else {
    SetThumbState(std::to_underlying(SliderState::Normal));
  }

  if (!mDraggingValue)
    return EventHandlerResult::Default;

  const auto thumbRange = layout.mThumbMax - layout.mThumbMin;

  const auto visualOffset = isHorizontal
    ? std::clamp(mouseOffset - layout.mThumbMin, 0.0f, thumbRange)
    : std::clamp(layout.mThumbMax - mouseOffset, 0.0f, thumbRange);

  const auto ratio = visualOffset / thumbRange;

  const auto valueOffset = ratio * (mMaximum - mMinimum);
  mDraggingValue = std::clamp(mMinimum + valueOffset, mMinimum, mMaximum);
  this->SetValue(this->GetSnappedValue(*mDraggingValue));
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult Slider::OnMouseHover(const MouseEvent& e) {
  const auto isHorizontal = mOrientation == Orientation::Horizontal;

  const auto fullLength = isHorizontal
    ? YGNodeLayoutGetWidth(GetLayoutNode())
    : YGNodeLayoutGetHeight(GetLayoutNode());
  const auto layout = GetLayout(fullLength);
  const auto yoga = GetLayoutNode();
  const auto thumbCenter = isHorizontal ? Point {
    layout.mThumbPoint,
    YGNodeLayoutGetHeight(yoga) / 2,
  } : Point {
    YGNodeLayoutGetWidth(yoga) / 2,
    YGNodeLayoutGetHeight(yoga) - layout.mThumbPoint,
  };
  const auto thumb = Rect::FromCenterAndSize(
    thumbCenter, {SliderThumbLength, SliderThumbLength});

  if (thumb.ContainsPoint(e.GetPosition())) {
    mWasThumbStationaryHovered = true;
  }

  return Widget::OnMouseHover(e);
}

void Slider::SetThumbState(const uint8_t raw) {
  if (mThumbState == raw) {
    return;
  }
  mThumbState = raw;

  const auto setInnerThumbScale = [this](const float scale) {
    const auto now = std::chrono::steady_clock::now();
    mInnerThumbScale.TransitionTo(
      scale,
      SliderInnerThumbScaleAnimationEasingFunction,
      now,
      now,
      now + SliderInnerThumbScaleAnimationDuration);
  };
  switch (static_cast<SliderState>(raw)) {
    case SliderState::Normal:
      setInnerThumbScale(SliderInnerThumbSize);
      break;
    case SliderState::Disabled:
      setInnerThumbScale(SliderInnerThumbScaleDisabled);
      break;
    case SliderState::Hover:
      setInnerThumbScale(SliderInnerThumbScalePointerOver);
      break;
    case SliderState::Active:
      setInnerThumbScale(SliderInnerThumbScalePressed);
      break;
  }
}

void Slider::OnMouseLeave(const MouseEvent& e) {
  if (!IsDisabled()) {
    SetThumbState(std::to_underlying(SliderState::Normal));
  }
  Widget::OnMouseLeave(e);
}

Widget::ComputedStyleFlags Slider::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  if (IsDisabled()) {
    SetThumbState(std::to_underlying(SliderState::Disabled));
  }
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableHoverState
    | ComputedStyleFlags::InheritableActiveState;
}

Widget::EventHandlerResult Slider::OnKeyPress(const KeyPressEvent& e) {
  if (IsDisabled()) {
    return Widget::OnKeyPress(e);
  }

  const auto step = (mStepFrequency > std::numeric_limits<float>::epsilon())
    ? mStepFrequency
    : (mMaximum - mMinimum) / 10;

  using enum KeyCode;
  switch (e.mKeyCode) {
    case Key_LeftArrow:
    case Key_DownArrow:
      mDidReceiveKeyboardInput = true;
      SetValue(mValue - step);
      return EventHandlerResult::StopPropagation;
    case Key_UpArrow:
    case Key_RightArrow:
      mDidReceiveKeyboardInput = true;
      SetValue(mValue + step);
      return EventHandlerResult::StopPropagation;
    case Key_Home:
      mDidReceiveKeyboardInput = true;
      SetValue(mMinimum);
      return EventHandlerResult::StopPropagation;
    case Key_End:
      mDidReceiveKeyboardInput = true;
      SetValue(mMaximum);
      return EventHandlerResult::StopPropagation;
    default:
      return Widget::OnKeyPress(e);
  }
}
FrameRateRequirement Slider::GetFrameRateRequirement() const noexcept {
  if (mInnerThumbScale.mEndTime > std::chrono::steady_clock::now()) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return Widget::GetFrameRateRequirement();
}

void Slider::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  Widget::PaintOwnContent(renderer, rect, style);

  const auto isHorizontal = mOrientation == Orientation::Horizontal;

  const auto length = isHorizontal ? rect.GetWidth() : rect.GetHeight();
  const auto layout = GetLayout(length);

  const auto [centerX, centerY] = rect.GetCenter();
  const auto origin = (isHorizontal ? rect.GetLeft() : rect.GetBottom());
  const auto makeCenteredPoint = [=](const float offset) {
    if (isHorizontal) {
      return Point {origin + offset, centerY};
    }
    return Point {centerX, origin - offset};
  };

  const auto brushes = [this]() -> SliderBrushes {
    if (IsDisabled()) {
      return GetSliderBrushes(SliderState::Disabled);
    }
    if (IsActive()) {
      return GetSliderBrushes(SliderState::Active);
    }
    if (IsHovered()) {
      return GetSliderBrushes(SliderState::Hover);
    }

    return GetSliderBrushes(SliderState::Normal);
  }();

  const auto trackThickness = *SliderTrackThemeHeight->Resolve();
  const auto thumbRange = layout.mThumbMax - layout.mThumbMin;

  // MIN_TICKMARK_GAP from WinUI3 TickBar_Partial.hpp
  static constexpr auto MinimumTickMarkGap = 20.0f;
  if (
    mTickFrequency > std::numeric_limits<float>::epsilon()
    && thumbRange >= MinimumTickMarkGap
    && mTickFrequency <= (mMaximum - mMinimum) / 2) {
    auto tickCount = std::lround((mMaximum - mMinimum) / mTickFrequency) + 1;
    const auto maximumTickCount
      = std::floor(thumbRange / MinimumTickMarkGap) + 1;

    std::size_t divisor = 1;
    while (tickCount > maximumTickCount) {
      ++divisor;
      tickCount
        = std::lround((mMaximum - mMinimum) / (mTickFrequency * divisor)) + 1;
    }
    const auto pixelsPerTick = thumbRange / (tickCount - 1);

    if (pixelsPerTick <= thumbRange) {
      FUI_ASSERT(pixelsPerTick >= MinimumTickMarkGap);
      const auto increment
        = isHorizontal ? Point {pixelsPerTick, 0} : Point {0, -pixelsPerTick};

      const auto distanceFromCenterline = (trackThickness / 2) + 4;
      const auto offsetFromCenter = isHorizontal
        ? Point {0, distanceFromCenterline}
        : Point {distanceFromCenterline, 0};
      const auto vector = isHorizontal
        ? Point {0, SliderOutsideTickBarThemeHeight}
        : Point {SliderOutsideTickBarThemeHeight, 0};
      const auto singlePixel = isHorizontal ? Point {0, 1} : Point {1, 0};

      const auto firstTickPoint = makeCenteredPoint(layout.mThumbMin);
      for (auto i = 0; i < tickCount; ++i) {
        const auto it = (i == (tickCount - 1))
          ? makeCenteredPoint(layout.mThumbMax)
          : (firstTickPoint + (increment * i));
        {
          const auto begin = it + offsetFromCenter;
          const auto end = begin + vector;
          renderer->DrawLine(brushes.mTickBarFill, begin, end);
        }
        {
          // When the thumb is perfectly on top of the tick mark,
          // the tick mark above (horizontal) or to the left (vertical) should
          // *just* be visible, but the one below or to the right should not.
          const auto begin = it - (offsetFromCenter + singlePixel);
          const auto end = begin - vector;
          renderer->DrawLine(brushes.mTickBarFill, begin, end);
        }
      }
    }
  }

  // Draw track
  {
    const auto radius
      = std::min(SliderTrackCornerRadius.GetUniformValue(), trackThickness / 2);

    auto lineFrom = makeCenteredPoint(layout.mTrackStart + radius);
    auto lineTo = makeCenteredPoint(layout.mTrackEnd - radius);

    if (isHorizontal) {
      renderer->PushClipRect({
        rect.GetTopLeft(),
        Size {layout.mThumbPoint, rect.GetHeight()},
      });
      renderer->DrawLine(
        brushes.mTrackValueFill,
        lineFrom,
        lineTo,
        trackThickness,
        StrokeCap::Round);
      renderer->PopClipRect();
      renderer->PushClipRect({
        rect.GetTopLeft() + Point {layout.mThumbPoint, 0},
        rect.GetBottomRight(),
      });
      renderer->DrawLine(
        brushes.mTrackFill, lineFrom, lineTo, trackThickness, StrokeCap::Round);
      renderer->PopClipRect();
    } else {
      std::swap(lineFrom, lineTo);
      renderer->PushClipRect({
        rect.GetTopLeft(),
        Size {rect.GetWidth(), length - layout.mThumbPoint},
      });
      renderer->DrawLine(
        brushes.mTrackFill, lineFrom, lineTo, trackThickness, StrokeCap::Round);
      renderer->PopClipRect();
      renderer->PushClipRect({
        rect.GetTopLeft() + Point {0, length - layout.mThumbPoint},
        rect.GetBottomRight(),
      });
      renderer->DrawLine(
        brushes.mTrackValueFill,
        lineFrom,
        lineTo,
        trackThickness,
        StrokeCap::Round);
      renderer->PopClipRect();
    }
  }

  const auto thumbCenter = makeCenteredPoint(layout.mThumbPoint);

  /* SliderThumbLength
   *   + 2px negative margin on both sides (harcoded in the XAML)
   *   - 1px for stroke thickness
   */
  static constexpr auto OuterThumbRadius = SliderThumbLength + 3.0f;
  const auto outerThumb = Rect::FromCenterAndSize(
    thumbCenter, {OuterThumbRadius, OuterThumbRadius});
  renderer->FillEllipse(brushes.mOuterThumbFill, outerThumb);
  renderer->StrokeEllipse(brushes.mOuterThumbStroke, outerThumb);

  const auto innerThumbScale = mInnerThumbScale.Evaluate(
    SliderInnerThumbScaleAnimationEasingFunction,
    std::chrono::steady_clock::now());
  const auto innerThumb = Rect::FromCenterAndSize(
    thumbCenter,
    {
      std::roundf(SliderInnerThumbWidth * innerThumbScale),
      std::roundf(SliderInnerThumbHeight * innerThumbScale),
    });
  renderer->FillEllipse(
    GetSliderInnerThumbBrush(static_cast<SliderState>(mThumbState)),
    innerThumb);
}

void Slider::SetTickFrequency(const float frequency) {
  if (frequency < 0.0f) [[unlikely]] {
    throw std::logic_error("Tick frequency must be positive");
  }
  if (frequency < std::numeric_limits<float>::epsilon()) {
    mTickFrequency = {};
  } else {
    mTickFrequency = frequency;
  }
}

}// namespace FredEmmott::GUI::Widgets