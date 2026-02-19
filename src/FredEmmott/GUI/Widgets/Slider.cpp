// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/Slider.hpp>
#include <FredEmmott/GUI/events/KeyCode.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <algorithm>

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace StaticTheme::Slider;
using namespace StaticTheme::Common;

auto& SliderBaseStyle() {
  static const ImmutableStyle ret {
    Style()
      .Height(SliderHorizontalHeight)
      .MinHeight(SliderHorizontalHeight)
      .FlexDirection(YGFlexDirectionRow)
      .AlignContent(YGAlignCenter)
      .AlignItems(YGAlignCenter)};
  return ret;
}

auto& TrackStyle() {
  static const ImmutableStyle ret {
    Style()
      .FlexGrow(1)
      .Height(SliderTrackThemeHeight)
      .BackgroundColor(SliderTrackBackgroundThemeBrush)
      .BorderRadius(SliderTrackCornerRadius)
      .MarginTop(SliderPreContentMargin)
      .MarginBottom(SliderPostContentMargin)
      .And(
        PseudoClasses::Hover,
        Style().BackgroundColor(SliderTrackPointerOverBackgroundThemeBrush))};
  return ret;
}

auto& OuterThumbStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignContent(YGAlignCenter)
      .AlignItems(YGAlignCenter)
      .JustifyContent(YGJustifyCenter)
      .Position(YGPositionTypeAbsolute)
      .Width(SliderHorizontalThumbWidth)
      .Height(SliderHorizontalThumbHeight)
      .BackgroundColor(SliderOuterThumbBackground)
      .BorderColor(SliderThumbBorderBrush)
      .BorderRadius(SliderThumbCornerRadius)
      .BorderWidth(1)};
  return ret;
}

auto& InnerThumbStyle() {
  static constexpr auto ScaleAnimation = CubicBezierStyleTransition(
    {}, ControlFastAnimationDuration, ControlFastOutSlowInKeySpline);
  static constexpr auto NormalScale = 0.86f;
  static constexpr auto PointerOverScale = 1.167;
  static constexpr auto PressedScale = 0.71f;
  static constexpr auto DisabledScale = PointerOverScale;

  static const ImmutableStyle ret {
    Style()
      .Width(SliderInnerThumbWidth)
      .Height(SliderInnerThumbHeight)
      .BackgroundColor(SliderThumbBackground)
      .BorderRadius(SliderThumbCornerRadius)
      .ScaleX(NormalScale, ScaleAnimation)
      .ScaleY(NormalScale, ScaleAnimation)
      .TransformOriginX(0.5f)
      .TransformOriginY(0.5f)
      .And(
        PseudoClasses::Hover,
        Style()
          .BackgroundColor(SliderThumbBackgroundPointerOver)
          .ScaleX(PointerOverScale, ScaleAnimation)
          .ScaleY(PointerOverScale, ScaleAnimation))
      .And(
        PseudoClasses::Disabled,
        Style()
          .BackgroundColor(SliderThumbBackgroundDisabled)
          .ScaleX(DisabledScale, ScaleAnimation)
          .ScaleY(DisabledScale, ScaleAnimation))
      .And(
        PseudoClasses::Active,
        Style()
          .BackgroundColor(SliderThumbBackgroundPressed)
          .ScaleX(PressedScale, ScaleAnimation)
          .ScaleY(PressedScale, ScaleAnimation))};
  return ret;
}

}// namespace

Slider::Slider(const std::size_t id)
  : Widget(id, StyleClass::Make("Slider"), SliderBaseStyle()) {
  mTrack = new Widget({}, StyleClass::Make("Slider/Track"), TrackStyle());
  mOuterThumb
    = new Widget({}, StyleClass::Make("Slider/Thumb"), OuterThumbStyle());
  mInnerThumb
    = new Widget({}, StyleClass::Make("Slider/Thumb/Inner"), InnerThumbStyle());
  mOuterThumb->SetChildren({mInnerThumb});

  this->SetChildren({mTrack, mOuterThumb});
}

void Slider::SetValue(const float value) {
  mValue = std::clamp(value, mMin, mMax);
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
  mMin = min;
  mMax = max;
  mValue = std::clamp(mValue, mMin, mMax);
}

void Slider::UpdateLayout() {
  Widget::UpdateLayout();
  UpdateThumbPosition();
}

void Slider::UpdateThumbPosition() {
  const auto renderValue = mDraggingValue.value_or(mValue);
  const float ratio = (renderValue - mMin) / (mMax - mMin);

  const float range = YGNodeLayoutGetWidth(mTrack->GetLayoutNode())
    - SliderHorizontalThumbWidth;
  mOuterThumb->SetMutableStyles(Style().Left(ratio * range));

  // This is not generally a safe assumption, but as we're built against
  // vendored-in copies of the WinUI3 XAML files for the slider which define
  // these brushes as solid color, we know that we're not going to have this
  // suddenly fail at runtime.
  //
  const auto makeValueFill
    = [ratio](const auto& valueFill, const auto& trackFill) {
        const auto valueColor = *valueFill->Resolve()->GetSolidColor();
        const auto trackColor = *trackFill->Resolve()->GetSolidColor();
        using Stop = LinearGradientBrush::Stop;
        return LinearGradientBrush(
          LinearGradientBrush::MappingMode::RelativeToBoundingBox,
          {0, 0},
          {1, 0},
          {
            Stop {0.0f, valueColor},
            Stop {ratio, valueColor},
            Stop {ratio, trackColor},
            Stop {1.0f, trackColor},
          });
      };
  mTrack->SetMutableStyles(
    Style()
      .BackgroundColor(makeValueFill(SliderTrackValueFill, SliderTrackFill))
      .And(
        PseudoClasses::Hover,
        Style().BackgroundColor(makeValueFill(
          SliderTrackValueFillPointerOver, SliderTrackFillPointerOver)))
      .And(
        PseudoClasses::Active,
        Style().BackgroundColor(
          makeValueFill(SliderTrackValueFillPressed, SliderTrackFillPressed)))
      .And(
        PseudoClasses::Disabled,
        Style().BackgroundColor(makeValueFill(
          SliderTrackValueFillDisabled, SliderTrackFillDisabled))));
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
    mDraggingValue.reset();
    const auto snapFrequency
      = (mSnapTo == SnapTo::Steps) ? mStepFrequency : mTickFrequency;
    if (snapFrequency < std::numeric_limits<float>::epsilon()) {
      mValue = *mDraggingValue;
    } else {
      const auto offset = *mDraggingValue - mMin;
      mValue = mMin + (std::round(offset / snapFrequency) * snapFrequency);
    }
    mValue = std::clamp(mValue, mMin, mMax);
    mDraggingValue.reset();
    mChanged = true;
  }
  std::ignore = Widget::OnMouseButtonRelease(e);
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult Slider::OnMouseMove(const MouseEvent& event) {
  if (!mDraggingValue)
    return EventHandlerResult::Default;

  const auto trackWidth = YGNodeLayoutGetWidth(mTrack->GetLayoutNode());
  const auto usableWidth = trackWidth - SliderHorizontalThumbWidth;
  const float ratio = std::clamp(
    (event.GetPosition().mX - (SliderHorizontalThumbWidth / 2)) / usableWidth,
    0.0f,
    1.0f);

  const auto offset = (ratio * (mMax - mMin));
  mDraggingValue = std::clamp(mMin + offset, mMin, mMax);
  return EventHandlerResult::StopPropagation;
}

Widget::ComputedStyleFlags Slider::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
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
    : (mMax - mMin) / 10;

  using enum KeyCode;
  switch (e.mKeyCode) {
    case Key_LeftArrow:
    case Key_DownArrow:
      SetValue(mValue - step);
      return EventHandlerResult::StopPropagation;
    case Key_UpArrow:
    case Key_RightArrow:
      SetValue(mValue + step);
      return EventHandlerResult::StopPropagation;
    case Key_Home:
      SetValue(mMin);
      return EventHandlerResult::StopPropagation;
    case Key_End:
      SetValue(mMax);
      return EventHandlerResult::StopPropagation;
    default:
      return Widget::OnKeyPress(e);
  }
}

void Slider::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style& style) const {
  Widget::PaintOwnContent(renderer, rect, style);

  if (!mTickFrequency) {
    return;
  }

  constexpr auto HalfThumb = SliderHorizontalThumbWidth / 2;
  const auto valueRange = (mMax - mMin);
  const auto leftMost = rect.GetLeft() + HalfThumb;
  const auto rightMost = rect.GetRight() - HalfThumb;
  const auto tickSpacing
    = ((rightMost - leftMost) / valueRange) * mTickFrequency;

  const auto yMid = rect.GetTop() + (rect.GetHeight() / 2);

  const auto y1 = yMid - HalfThumb;
  const auto y2 = y1 + SliderOutsideTickBarThemeHeight;
  const auto y3 = yMid + HalfThumb;
  const auto y4 = y3 - SliderOutsideTickBarThemeHeight;
  const auto& brush
    = *(this->IsDisabled() ? SliderTickBarFillDisabled : SliderTickBarFill)
         ->Resolve();

  for (float x = leftMost;
       x <= rightMost + std::numeric_limits<float>::epsilon();
       x += tickSpacing) {
    renderer->DrawLine(brush, {x, y1}, {x, y2});
    renderer->DrawLine(brush, {x, y3}, {x, y4});
  }
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