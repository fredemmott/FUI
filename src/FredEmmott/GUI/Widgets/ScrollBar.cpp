// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBar.hpp"

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/detail/Widget/ScrollBar.hpp>

#include "FredEmmott/GUI/Immediate/Button.hpp"
#include "Label.hpp"
#include "ScrollBarButton.hpp"
#include "ScrollBarThumb.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme::ScrollBar;
using namespace ScrollBarDetail;

namespace {

constexpr LiteralStyleClass ScrollBarStyleClass {"ScrollBar"};
constexpr LiteralStyleClass ScrollBarTrackStyleClass {"ScrollBarTrack"};

auto BaseStyles() {
  using namespace PseudoClasses;
  return Style()
    .BackgroundColor(ScrollBarBackground)
    .And(Disabled, Style().BackgroundColor(ScrollBarBackgroundDisabled))
    .And(Hover, Style().BackgroundColor(ScrollBarBackgroundPointerOver));
}

auto& HorizontalStyles() {
  static const ImmutableStyle ret {
    BaseStyles() + Style().FlexDirection(YGFlexDirectionRow),
  };
  return ret;
}

auto& VerticalStyles() {
  static const ImmutableStyle ret {
    BaseStyles() + Style().FlexDirection(YGFlexDirectionColumn),
  };
  return ret;
}

auto& HorizontalTrackStyle() {
  static const ImmutableStyle ret {
    Style()
      .Display(YGDisplayFlex)
      .FlexGrow(1)
      .FlexDirection(YGFlexDirectionRow)
      .MarginLeft(2.f)
      .MarginRight(2.f),
  };
  return ret;
}

auto& VerticalTrackStyle() {
  static const ImmutableStyle ret {
    Style()
      .Display(YGDisplayFlex)
      .FlexGrow(1)
      .FlexDirection(YGFlexDirectionColumn)
      .MarginBottom(2.f)
      .MarginTop(2.f),
  };
  return ret;
}

auto MakeSmallChangeBaseStyles() {
  // Hardcoded in XAML
  constexpr auto SmallPressedAnimation
    = LinearStyleTransition(std::chrono::milliseconds(16));
  using namespace PseudoClasses;
  return Style()
    .Color(ScrollBarButtonArrowForeground)
    .Opacity(0)
    .Font(
      ResolveGlyphFont(SystemFont::Body)
        .WithSize(ScrollBarButtonArrowIconFontSize),
      !important)
    .ScaleX(1, SmallPressedAnimation)
    .ScaleY(1, SmallPressedAnimation)
    .TranslateY(0, SmallPressedAnimation)
    .TranslateX(0, SmallPressedAnimation)
    .Top(0, SmallPressedAnimation)
    .And(Hover, Style().Color(ScrollBarButtonArrowForegroundPointerOver))
    .And(
      Active,
      Style()
        .Color(ScrollBarButtonArrowForegroundPressed)
        .ScaleX(ScrollBarButtonArrowScalePressed)
        .ScaleY(ScrollBarButtonArrowScalePressed));
}

auto& HorizontalSmallChangeStyles() {
  using namespace PseudoClasses;
  static const ImmutableStyle ret {
    MakeSmallChangeBaseStyles() + Style().And(Active, Style().TranslateY(1.f)),
  };
  return ret;
}

auto& VerticalSmallChangeStyles() {
  static const ImmutableStyle ret {
    MakeSmallChangeBaseStyles() + Style().Left(2.f),
  };
  return ret;
}

auto& LargeChangeStyles() {
  static const ImmutableStyle ret {
    Style().FlexGrow(1),
  };
  return ret;
}
}// namespace

ScrollBar::ScrollBar(const std::size_t id, const Orientation orientation)
  : ScrollBar(
      id,
      (orientation == Orientation::Horizontal) ? HorizontalStyles()
                                               : VerticalStyles(),
      orientation) {}

ScrollBar::ScrollBar(
  const std::size_t id,
  const ImmutableStyle& style,
  const Orientation orientation)
  : Widget(id, style, {ScrollBarStyleClass}),
    mOrientation(orientation) {
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  constexpr auto leftGlyph = "\uedd9";
  constexpr auto rightGlyph = "\uedda";
  constexpr auto upGlyph = "\ueddb";
  constexpr auto downGlyph = "\ueddc";

  const auto isHorizontal = (orientation == Orientation::Horizontal);

  this->ChangeDirectChildren([this, isHorizontal] {
    const auto& SmallChangeStyles = isHorizontal
      ? HorizontalSmallChangeStyles()
      : VerticalSmallChangeStyles();
    mSmallDecrement.reset(new ScrollBarButton(
      0,
      SmallChangeStyles,
      nullptr,
      std::bind_front(
        &ScrollBar::ScrollBarButtonTick,
        this,
        ButtonTickKind::SmallDecrement)));
    mTrack.reset(new Widget(
      0,
      (isHorizontal) ? HorizontalTrackStyle() : VerticalTrackStyle(),
      {ScrollBarTrackStyleClass}));
    mSmallIncrement.reset(new ScrollBarButton(
      0,
      SmallChangeStyles,
      nullptr,
      std::bind_front(
        &ScrollBar::ScrollBarButtonTick,
        this,
        ButtonTickKind::SmallIncrement)));
  });

  mLargeDecrement = new ScrollBarButton(
    0,
    LargeChangeStyles(),
    std::bind_front(
      &ScrollBar::ScrollBarButtonDown, this, ButtonTickKind::LargeDecrement),
    std::bind_front(
      &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::LargeDecrement));
  mThumb = new ScrollBarThumb(orientation, 0);
  mLargeIncrement = new ScrollBarButton(
    0,
    LargeChangeStyles(),
    std::bind_front(
      &ScrollBar::ScrollBarButtonDown, this, ButtonTickKind::LargeIncrement),
    std::bind_front(
      &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::LargeIncrement));
  mTrack->SetChildren({mLargeDecrement, mThumb, mLargeIncrement});
  mThumb->OnDrag(std::bind_front(&ScrollBar::OnThumbDrag, this));

  switch (orientation) {
    case Orientation::Vertical:
      mSmallDecrement->SetText(upGlyph);
      mSmallIncrement->SetText(downGlyph);
      break;
    case Orientation::Horizontal:
      mSmallDecrement->SetText(leftGlyph);
      mSmallIncrement->SetText(rightGlyph);
      break;
  }

  this->UpdateLayout();
}

ImmutableStyle ScrollBar::MakeImmutableStyle(
  const Orientation orientation,
  const Style& mixin) {
  using enum Orientation;
  switch (orientation) {
    case Horizontal:
      return ImmutableStyle {HorizontalStyles().Get() + mixin};
    case Vertical:
      return ImmutableStyle {VerticalStyles().Get() + mixin};
  }
  std::unreachable();
}
ScrollBar::~ScrollBar() = default;

WidgetList ScrollBar::GetDirectChildren() const noexcept {
  return {
    mSmallDecrement.get(),
    mTrack.get(),
    mSmallIncrement.get(),
  };
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