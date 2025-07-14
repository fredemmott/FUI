// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBar.hpp"

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>

#include "FredEmmott/GUI/Immediate/Button.hpp"
#include "Label.hpp"
#include "ScrollBarButton.hpp"
#include "ScrollBarThumb.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme::ScrollBar;

namespace {

const auto ScrollBarStyleClass = StyleClass::Make("ScrollBar");
const auto ScrollBarTrackStyleClass = StyleClass::Make("ScrollBarTrack");

const auto ContractAnimation = CubicBezierStyleTransition(
  ScrollBarContractBeginTime,
  ScrollBarOpacityChangeDuration,
  StaticTheme::Common::ControlFastOutSlowInKeySpline);
const auto ExpandAnimation = CubicBezierStyleTransition(
  ScrollBarExpandBeginTime,
  ScrollBarOpacityChangeDuration,
  StaticTheme::Common::ControlFastOutSlowInKeySpline);

}// namespace

ScrollBar::ScrollBar(std::size_t id, Orientation orientation)
  : Widget(id, {ScrollBarStyleClass}),
    mOrientation(orientation),
    mBuiltinStyles(GetBuiltinStylesForOrientation()) {
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  constexpr auto leftGlyph = "\uedd9";
  constexpr auto rightGlyph = "\uedda";
  constexpr auto upGlyph = "\ueddb";
  constexpr auto downGlyph = "\ueddc";

  this->ChangeDirectChildren([this] {
    mSmallDecrement.reset(new ScrollBarButton(
      nullptr,
      std::bind_front(
        &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::SmallDecrement),
      0));
    mTrack.reset(new Widget(0, {ScrollBarTrackStyleClass}));
    mSmallIncrement.reset(new ScrollBarButton(
      nullptr,
      std::bind_front(
        &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::SmallIncrement),
      0));
  });

  mLargeDecrement = new ScrollBarButton(
    std::bind_front(
      &ScrollBar::ScrollBarButtonDown, this, ButtonTickKind::LargeDecrement),
    std::bind_front(
      &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::LargeDecrement),
    0);
  mThumb = new ScrollBarThumb(0);
  mLargeIncrement = new ScrollBarButton(
    std::bind_front(
      &ScrollBar::ScrollBarButtonDown, this, ButtonTickKind::LargeIncrement),
    std::bind_front(
      &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::LargeIncrement),
    0);
  mTrack->SetChildren({mLargeDecrement, mThumb, mLargeIncrement});
  mTrack->SetBuiltInStyles(
    Style()
      .Display(YGDisplayFlex)
      .FlexDirection(
        (orientation == Orientation::Horizontal) ? YGFlexDirectionRow
                                                 : YGFlexDirectionColumn)
      .FlexGrow(1)
      .MarginBottom((orientation == Orientation::Horizontal) ? 0 : 2.0f)
      .MarginLeft((orientation == Orientation::Horizontal) ? 2.0f : 0)
      .MarginRight((orientation == Orientation::Horizontal) ? 2.0f : 0)
      .MarginTop((orientation == Orientation::Horizontal) ? 0 : 2.0f));
  mThumb->OnDrag(std::bind_front(&ScrollBar::OnThumbDrag, this));

  // Hardcoded in XAML
  constexpr auto SmallPressedAnimation
    = LinearStyleTransition(std::chrono::milliseconds(16));

  using namespace PseudoClasses;
  const Style SmallChangeStyles
    = Style()
        .Color(ScrollBarButtonArrowForeground)
        .Font(
          ResolveGlyphFont(SystemFont::Body)
            .WithSize(ScrollBarButtonArrowIconFontSize),
          !important)
        .Left((mOrientation == Orientation::Horizontal) ? 0 : 2.0f)
        .Opacity(0)
        .ScaleX(1, SmallPressedAnimation)
        .ScaleY(1, SmallPressedAnimation)
        .Top(0, SmallPressedAnimation)
        .TranslateY(0, SmallPressedAnimation)
        .And(Hover, Style().Color(ScrollBarButtonArrowForegroundPointerOver))
        .And(
          Active,
          Style()
            .Color(ScrollBarButtonArrowForegroundPressed)
            .ScaleX(ScrollBarButtonArrowScalePressed)
            .ScaleY(ScrollBarButtonArrowScalePressed)
            .TranslateY((orientation == Orientation::Vertical) ? 0 : 1.0f));

  mSmallDecrement->SetBuiltInStyles({SmallChangeStyles});
  mSmallIncrement->SetBuiltInStyles({SmallChangeStyles});

  static const auto sLargeChangeStyles = Style().FlexGrow(1);
  mLargeDecrement->SetBuiltInStyles({sLargeChangeStyles});
  mLargeIncrement->SetBuiltInStyles({sLargeChangeStyles});

  const bool isHorizontal = (orientation == Orientation::Horizontal);
  auto thumbStyles
    = Style()
        .BackgroundColor(ScrollBarThumbFill)
        .BorderRadius(ScrollBarCornerRadius)
        .Height(
          static_cast<float>(
            isHorizontal ? ScrollBarHorizontalThumbMinHeight
                         : ScrollBarVerticalThumbMinHeight),
          ContractAnimation)
        .Width(
          static_cast<float>(
            isHorizontal ? ScrollBarHorizontalThumbMinWidth
                         : ScrollBarVerticalThumbMinWidth),
          ExpandAnimation)
        .And(Disabled, Style().BackgroundColor(ScrollBarThumbFillDisabled))
        .And(Hover, Style().BackgroundColor(ScrollBarThumbFillPointerOver));

  switch (orientation) {
    case Orientation::Vertical:
      thumbStyles.Width()
        = {ScrollBarHorizontalThumbMinWidth, ContractAnimation};
      mSmallDecrement->SetText(upGlyph);
      mSmallIncrement->SetText(downGlyph);
      break;
    case Orientation::Horizontal:
      thumbStyles.Height()
        = {ScrollBarHorizontalThumbMinHeight, ContractAnimation};
      mSmallDecrement->SetText(leftGlyph);
      mSmallIncrement->SetText(rightGlyph);
      break;
  }

  mThumb->SetBuiltInStyles({thumbStyles});
  this->UpdateLayout();
}

ScrollBar::~ScrollBar() = default;

WidgetList ScrollBar::GetDirectChildren() const noexcept {
  return {
    mSmallDecrement.get(),
    mTrack.get(),
    mSmallIncrement.get(),
  };
}

Style ScrollBar::GetBuiltInStyles() const {
  return mBuiltinStyles;
}

Widget::ComputedStyleFlags ScrollBar::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  const bool hovered = (state & StateFlags::Hovered) == StateFlags::Hovered;
  auto smallChangeStyles
    = Style()
        .Opacity(
          (hovered ? 1.0f : 0.0f),
          (hovered ? ExpandAnimation : ContractAnimation))
        .Top((mOrientation == Orientation::Vertical) ? 0 : -3.0f);
  mSmallDecrement->ReplaceExplicitStyles(smallChangeStyles);
  mSmallIncrement->ReplaceExplicitStyles(smallChangeStyles);

  if (mOrientation == Orientation::Horizontal) {
    mThumb->AddExplicitStyles(
      Style().Height(
        static_cast<float>(
          hovered ? ScrollBarSize : ScrollBarHorizontalThumbMinHeight),
        hovered ? ExpandAnimation : ContractAnimation));
  } else {
    mThumb->AddExplicitStyles(
      Style().Width(
        static_cast<float>(
          hovered ? ScrollBarSize : ScrollBarVerticalThumbMinWidth),
        hovered ? ExpandAnimation : ContractAnimation));
  }

  return Widget::OnComputedStyleChange(style, state);
}

Style ScrollBar::GetBuiltinStylesForOrientation() const {
  using namespace PseudoClasses;
  static const auto sBaseStyles
    = Style()
        .BackgroundColor(ScrollBarBackground)
        .And(Disabled, Style().BackgroundColor(ScrollBarBackgroundDisabled))
        .And(Hover, Style().BackgroundColor(ScrollBarBackgroundPointerOver));

  static const auto sHorizontalStyles
    = sBaseStyles + Style().FlexDirection(YGFlexDirectionRow);
  static const auto sVerticalStyles
    = sBaseStyles + Style().FlexDirection(YGFlexDirectionColumn);

  return (mOrientation == Orientation::Horizontal) ? sHorizontalStyles
                                                   : sVerticalStyles;
}

void ScrollBar::ScrollBarButtonTick(ButtonTickKind kind) {
  const float min = (kind == ButtonTickKind::LargeDecrement)
    ? mLargeDecrementMin.value()
    : mMinimum;
  const float max = (kind == ButtonTickKind::LargeIncrement)
    ? mLargeIncrementMax.value()
    : mMaximum;

  float base = mThumbSize;
  if (base < std::numeric_limits<float>::epsilon()) {
    const auto measureFunc = (mOrientation == Orientation::Horizontal)
      ? &YGNodeLayoutGetWidth
      : &YGNodeLayoutGetHeight;
    const auto trackLen = measureFunc(mTrack->GetLayoutNode());
    const auto thumbLen = measureFunc(mThumb->GetLayoutNode());
    base = (thumbLen / trackLen) * (mMaximum - mMinimum);
  }

  float delta {};
  switch (kind) {
    case ButtonTickKind::SmallDecrement:
      delta = -base / 4;
      break;
    case ButtonTickKind::SmallIncrement:
      delta = base / 4;
      break;
    case ButtonTickKind::LargeDecrement:
      delta = -base;
      break;
    case ButtonTickKind::LargeIncrement:
      delta = base;
      break;
  }
  const float clamped = std::clamp(mValue + delta, min, max);
  this->SetValue(clamped, ChangeReason::Discrete);
}

void ScrollBar::ScrollBarButtonDown(ButtonTickKind kind, const Point& point) {
  const auto measureFun = (mOrientation == Orientation::Horizontal)
    ? &YGNodeLayoutGetWidth
    : &YGNodeLayoutGetHeight;
  const auto pointGetter
    = (mOrientation == Orientation::Horizontal) ? &Point::mX : &Point::mY;
  float rangeV {};

  Widget* button {nullptr};
  switch (kind) {
    case ButtonTickKind::LargeDecrement:
      button = mLargeDecrement;
      rangeV = mValue - mMinimum;
      break;
    case ButtonTickKind::LargeIncrement:
      button = mLargeIncrement;
      rangeV = mMaximum - mValue;
      break;
    default:
      __debugbreak();
      return;
  }

  const auto rangePixels = measureFun(button->GetLayoutNode());
  const auto deltaPixels = std::invoke(pointGetter, &point);
  const auto deltaV = (rangeV * deltaPixels) / rangePixels;

  switch (kind) {
    case ButtonTickKind::LargeDecrement:
      mLargeDecrementMin = mMinimum + deltaV;
      break;
    case ButtonTickKind::LargeIncrement:
      mLargeIncrementMax = mValue + deltaV;
      break;
    default:
      std::unreachable();
  }
}

void ScrollBar::UpdateLayout() {
  mLargeDecrement->ReplaceExplicitStyles(Style().FlexGrow(mValue - mMinimum));
  mThumb->AddExplicitStyles(Style().FlexGrow(mThumbSize));
  mLargeIncrement->ReplaceExplicitStyles(Style().FlexGrow(mMaximum - mValue));
}

void ScrollBar::OnThumbDrag(Point* deltaXY) {
  const auto rangeV = mMaximum - mMinimum;
  if (rangeV < std::numeric_limits<float>::epsilon()) {
    return;
  }

  const auto measureFun = (mOrientation == Orientation::Horizontal)
    ? &YGNodeLayoutGetWidth
    : &YGNodeLayoutGetHeight;
  const auto rangePixels = measureFun(mLargeDecrement->GetLayoutNode())
    + measureFun(mLargeIncrement->GetLayoutNode());

  const auto deltaPtr
    = (mOrientation == Orientation::Horizontal) ? &Point::mX : &Point::mY;
  const auto deltaPixels = std::invoke(deltaPtr, deltaXY);
  const auto valuePerPixel = rangeV / rangePixels;
  const auto deltaV = deltaPixels * valuePerPixel;

  const auto rawValue = mValue + deltaV;
  const auto clampedValue = std::clamp(rawValue, mMinimum, mMaximum);

  if (rawValue != clampedValue) {
    const auto fixPx = (clampedValue - rawValue) / valuePerPixel;
    std::invoke(deltaPtr, deltaXY) += fixPx;
  }

  this->SetValue(clampedValue, ChangeReason::Continuous);
}

void ScrollBar::SetMinimum(float value) {
  mMinimum = value;
  this->UpdateLayout();
}

float ScrollBar::GetMinimum() const {
  return mMinimum;
}

void ScrollBar::SetMaximum(float maximum) {
  mMaximum = maximum;
  if (mValue > mMaximum) {
    this->SetValue(maximum);
  }
  this->UpdateLayout();
}

float ScrollBar::GetMaximum() const {
  return mMaximum;
}

float ScrollBar::GetValue() const {
  return mValue;
}

void ScrollBar::SetValue(const float value) {
  this->SetValue(value, ChangeReason::Programmatic);
}

void ScrollBar::SetValue(const float value, const ChangeReason reason) {
  if (mValue == value) {
    return;
  }
  mValue = value;
  if (mValueChangedCallback) {
    mValueChangedCallback(value, reason);
  }
  this->UpdateLayout();
}

float ScrollBar::GetThumbSize() const {
  return mThumbSize;
}

void ScrollBar::OnValueChanged(ValueChangedCallback cb) {
  mValueChangedCallback = std::move(cb);
}

void ScrollBar::SetThumbSize(float value) {
  mThumbSize = value;
  this->UpdateLayout();
}

}// namespace FredEmmott::GUI::Widgets