// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/Slider.hpp>
#include <FredEmmott/GUI/events/KeyCode.hpp>
#include <FredEmmott/GUI/events/KeyEvent.hpp>
#include <algorithm>

#include "FredEmmott/utility/almost_equal.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace StaticTheme::Slider;
using namespace StaticTheme::Common;

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

auto& TrackBaseStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignItems(YGAlignCenter)
      .AlignSelf(YGAlignCenter)
      .BackgroundColor(SliderTrackBackgroundThemeBrush)
      .BorderRadius(SliderTrackCornerRadius)
      .FlexGrow(1)
      .MarginTop(SliderPreContentMargin)
      .MarginBottom(SliderPostContentMargin)
      .And(
        PseudoClasses::Hover,
        Style().BackgroundColor(SliderTrackPointerOverBackgroundThemeBrush))};
  return ret;
}

auto& HorizontalTrackStyle() {
  static const ImmutableStyle ret {
    TrackBaseStyle()
    + Style().Height(SliderTrackThemeHeight).FlexDirection(YGFlexDirectionRow)};
  return ret;
}

auto& VerticalTrackStyle() {
  static const ImmutableStyle ret {
    TrackBaseStyle()
    + Style()
        .Width(SliderTrackThemeHeight)
        .FlexDirection(YGFlexDirectionColumn)};
  return ret;
}

auto& OuterThumbBaseStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignItems(YGAlignCenter)
      .JustifyContent(YGJustifyCenter)
      .BackgroundColor(SliderOuterThumbBackground)
      .BorderColor(SliderThumbBorderBrush)
      .BorderRadius(SliderThumbCornerRadius)
      .BorderWidth(1)};
  return ret;
}

auto& HorizontalOuterThumbStyle() {
  static const ImmutableStyle ret {
    OuterThumbBaseStyle()
    + Style()
        .Width(SliderHorizontalThumbWidth)
        .Height(SliderHorizontalThumbHeight)};
  return ret;
}

auto& VerticalOuterThumbStyle() {
  static const ImmutableStyle ret {
    OuterThumbBaseStyle()
    + Style()
        .Width(SliderVerticalThumbWidth)
        .Height(SliderVerticalThumbHeight)};
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

Slider::Slider(const std::size_t id, const Orientation orientation)
  : Widget(
      id,
      LiteralStyleClass("Slider"),
      ((orientation == Orientation::Horizontal) ? HorizontalSliderStyle()
                                                : VerticalSliderStyle())),
    mOrientation(orientation) {
  const auto isHorizontal = orientation == Orientation::Horizontal;
  mTrack = new Widget(
    {},
    LiteralStyleClass("Slider/Track"),
    isHorizontal ? HorizontalTrackStyle() : VerticalTrackStyle());
  mOuterThumb = new Widget(
    {},
    LiteralStyleClass("Slider/Thumb"),
    isHorizontal ? HorizontalOuterThumbStyle() : VerticalOuterThumbStyle());
  mInnerThumb = new Widget(
    {}, LiteralStyleClass("Slider/Thumb/Inner"), InnerThumbStyle());
  mOuterThumb->SetChildren({mInnerThumb});

  mBeforeThumb = new Widget({}, LiteralStyleClass("Slider/Thumb/Spacer"), {});
  mAfterThumb = new Widget({}, LiteralStyleClass("Slider/Thumb/Spacer"), {});

  mTrack->SetChildren({
    mBeforeThumb,
    mOuterThumb,
    mAfterThumb,
  });

  this->SetChildren({mTrack});
  this->UpdateThumbPosition();
}

void Slider::SetValue(const float value) {
  if (utility::almost_equal(value, mValue)) {
    return;
  }
  mValue = std::clamp(value, mMin, mMax);
  this->UpdateThumbPosition();
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
  this->UpdateThumbPosition();
}

void Slider::UpdateThumbPosition() {
  const auto renderValue = mDraggingValue.value_or(mValue);
  const auto ratio = (renderValue - mMin) / (mMax - mMin);

  Point fillStart {0, 0};
  Point fillEnd {1, 0};

  // Handle dragging differently to dropping because:
  // - while we're dragging, we want to avoid re-layout every frame for
  //   performance, so we don't want to modify FlexGrow every frame, as that
  //   marks the layout as dirty, including all ancestors and descendants. So,
  //   we hide the spacers, and use Translate instead
  // - after drop, we want a layout that can be recalculated independently of
  //   this widget, e.g. while resizing parent, so we use ratio-based spacers
  //   with 0 Translate
  if (mDraggingValue) {
    mBeforeThumb->SetMutableStyles(Style().Display(YGDisplayNone));
    mAfterThumb->SetMutableStyles(Style().Display(YGDisplayNone));
  }

  if (mOrientation == Orientation::Horizontal) {
    if (mDraggingValue) {
      static constexpr auto ThumbLength = SliderHorizontalThumbWidth;
      const auto usableLength
        = YGNodeLayoutGetWidth(mTrack->GetLayoutNode()) - ThumbLength;
      mOuterThumb->SetMutableStyles(Style().TranslateX(ratio * usableLength));
    } else {
      mBeforeThumb->SetMutableStyles(
        Style().Display(YGDisplayFlex).FlexGrow(ratio));
      mAfterThumb->SetMutableStyles(
        Style().Display(YGDisplayFlex).FlexGrow(1.0f - ratio));
      mOuterThumb->SetMutableStyles(Style().TranslateX(0));
    }
  } else {
    fillStart = {0, 1};
    fillEnd = {0, 0};
    if (mDraggingValue) {
      static constexpr auto ThumbLength = SliderVerticalThumbHeight;
      const auto usableLength
        = YGNodeLayoutGetHeight(mTrack->GetLayoutNode()) - ThumbLength;
      mOuterThumb->SetMutableStyles(
        Style().TranslateY((1.0f - ratio) * usableLength));
    } else {
      mBeforeThumb->SetMutableStyles(
        Style().Display(YGDisplayFlex).FlexGrow(1.0f - ratio));
      mAfterThumb->SetMutableStyles(
        Style().Display(YGDisplayFlex).FlexGrow(ratio));
      mOuterThumb->SetMutableStyles(Style().TranslateY(0));
    }
  }

  // This is not generally a safe assumption, but as we're built against
  // vendored-in copies of the WinUI3 XAML files for the slider which define
  // these brushes as solid color, we know that we're not going to have this
  // suddenly fail at runtime.
  //
  const auto makeValueFill = [ratio, &fillStart, &fillEnd](
                               const auto& valueFill, const auto& trackFill) {
    const auto valueColor = *valueFill->Resolve()->GetSolidColor();
    const auto trackColor = *trackFill->Resolve()->GetSolidColor();
    using Stop = LinearGradientBrush::Stop;
    return LinearGradientBrush(
      LinearGradientBrush::MappingMode::RelativeToBoundingBox,
      fillStart,
      fillEnd,
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

float Slider::GetSnappedValue(float value) const noexcept {
  const auto snapFrequency
    = (mSnapTo == SnapTo::Steps) ? mStepFrequency : mTickFrequency;
  if (snapFrequency >= std::numeric_limits<float>::epsilon()) {
    const auto offset = value - mMin;
    value = mMin + (std::round(offset / snapFrequency) * snapFrequency);
  }
  return std::clamp(value, mMin, mMax);
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

    const auto release = wil::scope_exit([this] {
      mDraggingValue.reset();
      mDraggingTrackOffset.reset();
      this->UpdateThumbPosition();
    });

    mValue = this->GetSnappedValue(*mDraggingValue);
    mChanged = true;
  }
  std::ignore = Widget::OnMouseButtonRelease(e);
  return EventHandlerResult::StopPropagation;
}

float Slider::GetSnappedDraggingValue() const {
  return GetSnappedValue(*mDraggingValue);
}

Point Slider::GetTrackOriginOffset() const {
  const auto tl
    = mTrack->GetTopLeftCanvasPoint() - this->GetTopLeftCanvasPoint();
  return tl
    + Point {
      SliderHorizontalThumbWidth / 2,
      YGNodeLayoutGetHeight(mTrack->GetLayoutNode()) / 2};
}

float Slider::GetTrackLength() const {
  if (mOrientation == Orientation::Horizontal) {
    return YGNodeLayoutGetWidth(mTrack->GetLayoutNode());
  } else {
    return YGNodeLayoutGetHeight(mTrack->GetLayoutNode());
  }
}

Widget::EventHandlerResult Slider::OnMouseMove(const MouseEvent& event) {
  if (!mDraggingValue)
    return EventHandlerResult::Default;

  const auto ratio = [&event, this] {
    switch (mOrientation) {
      case Orientation::Horizontal: {
        const auto trackLength = YGNodeLayoutGetWidth(mTrack->GetLayoutNode());
        const auto usableLength = trackLength - SliderHorizontalThumbWidth;
        mDraggingTrackOffset = std::clamp(
          event.GetPosition().mX - (SliderHorizontalThumbWidth / 2),
          0.0f,
          usableLength);
        return *mDraggingTrackOffset / usableLength;
      }
      case Orientation::Vertical: {
        const auto trackLength = YGNodeLayoutGetHeight(mTrack->GetLayoutNode());
        const auto usableLength = trackLength - SliderVerticalThumbHeight;
        mDraggingTrackOffset = std::clamp(
          usableLength
            - (event.GetPosition().mY - (SliderVerticalThumbHeight / 2)),
          0.0f,
          usableLength);
        return *mDraggingTrackOffset / usableLength;
      }
    }
    std::unreachable();
  }();

  const auto valueOffset = ratio * (mMax - mMin);
  mDraggingValue = std::clamp(mMin + valueOffset, mMin, mMax);
  this->UpdateThumbPosition();
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

  float halfThumb, begin, end, valueMid;
  Point (*makePoint)(float i, float value) = nullptr;
  if (mOrientation == Orientation::Horizontal) {
    halfThumb = SliderHorizontalThumbWidth / 2;
    begin = rect.GetLeft() + halfThumb;
    end = rect.GetRight() - halfThumb;
    valueMid = rect.GetTop() + (rect.GetHeight() / 2);
    makePoint
      = [](const float i, const float value) { return Point {i, value}; };
  } else {
    halfThumb = SliderVerticalThumbHeight / 2;
    begin = rect.GetTop() + halfThumb;
    end = rect.GetBottom() - halfThumb;
    valueMid = rect.GetLeft() + (rect.GetWidth() / 2);
    makePoint
      = [](const float i, const float value) { return Point {value, i}; };
  }

  const auto valueRange = (mMax - mMin);
  const auto tickSpacing = ((end - begin) / valueRange) * mTickFrequency;

  const auto v1 = valueMid - halfThumb;
  const auto v2 = v1 + SliderOutsideTickBarThemeHeight;
  const auto v3 = valueMid + halfThumb;
  const auto v4 = v3 - SliderOutsideTickBarThemeHeight;
  const auto& brush
    = *(this->IsDisabled() ? SliderTickBarFillDisabled : SliderTickBarFill)
         ->Resolve();

  for (float i = begin; i <= end + std::numeric_limits<float>::epsilon();
       i += tickSpacing) {
    renderer->DrawLine(brush, makePoint(i, v1), makePoint(i, v2));
    renderer->DrawLine(brush, makePoint(i, v3), makePoint(i, v4));
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