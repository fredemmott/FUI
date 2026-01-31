// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Slider.hpp"

#include <FredEmmott/GUI/StaticTheme/Slider.hpp>
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
      .And(
        PseudoClasses::Hover,
        Style().BackgroundColor(SliderTrackPointerOverBackgroundThemeBrush))};
  return ret;
}

auto& ThumbStyle() {
  static const ImmutableStyle ret {
    Style()
      .Position(YGPositionTypeAbsolute)
      .Width(SliderHorizontalThumbWidth)
      .Height(SliderHorizontalThumbHeight)
      .BackgroundColor(SliderThumbBackground)
      .BorderRadius(SliderThumbCornerRadius)
      .And(
        PseudoClasses::Hover,
        Style().BackgroundColor(SliderThumbBackgroundPointerOver))
      .And(
        PseudoClasses::Active,
        Style().BackgroundColor(SliderThumbBackgroundPressed))};
  return ret;
}
}// namespace

Slider::Slider(const std::size_t id)
  : Widget(id, StyleClass::Make("Slider"), SliderBaseStyle()) {
  mTrack = new Widget({}, StyleClass::Make("SliderTrack"), TrackStyle());
  mThumb = new Widget({}, StyleClass::Make("SliderThumb"), ThumbStyle());

  mTrack->SetChildren({mThumb});
  this->SetDirectChildren({mTrack});
}

void Slider::SetValue(const float value) {
  mValue = value;
}

float Slider::GetValue() const {
  return mValue;
}

void Slider::SetRange(float min, float max) {
  mMin = min;
  mMax = max;
}

void Slider::UpdateLayout() {
  Widget::UpdateLayout();
  UpdateThumbPosition();
}

void Slider::UpdateThumbPosition() {
  const float ratio = (mValue - mMin) / (mMax - mMin);

  const float range = YGNodeLayoutGetWidth(mTrack->GetLayoutNode())
    - SliderHorizontalThumbWidth;
  mThumb->AddMutableStyles(Style().Left(ratio * range));
}

Widget::EventHandlerResult Slider::OnMouseButtonPress(const MouseEvent& event) {
  mIsDragging = true;
  StartMouseCapture();
  return OnMouseMove(event);
}

Widget::EventHandlerResult Slider::OnMouseButtonRelease(const MouseEvent&) {
  mIsDragging = false;
  EndMouseCapture();
  return EventHandlerResult::StopPropagation;
}

Widget::EventHandlerResult Slider::OnMouseMove(const MouseEvent& event) {
  if (!mIsDragging)
    return EventHandlerResult::Default;

  const auto trackWidth = YGNodeLayoutGetWidth(mTrack->GetLayoutNode());
  const auto usableWidth = trackWidth - SliderHorizontalThumbWidth;
  const float ratio = std::clamp(
    (event.GetPosition().mX - (SliderHorizontalThumbWidth / 2)) / usableWidth,
    0.0f,
    1.0f);

  mValue = mMin + (ratio * (mMax - mMin));
  UpdateThumbPosition();
  mChanged = true;
  return EventHandlerResult::StopPropagation;
}

Widget::ComputedStyleFlags Slider::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableHoverState
    | ComputedStyleFlags::InheritableActiveState;
}

}// namespace FredEmmott::GUI::Widgets