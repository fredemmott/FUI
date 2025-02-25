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

  // FIXME: callbacks
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
    nullptr,
    std::bind_front(
      &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::LargeDecrement),
    0);
  mThumb = new ScrollBarThumb(0);
  mLargeIncrement = new ScrollBarButton(
    nullptr,
    std::bind_front(
      &ScrollBar::ScrollBarButtonTick, this, ButtonTickKind::LargeIncrement),
    0);
  mTrack->SetChildren({mLargeDecrement, mThumb, mLargeIncrement});
  mTrack->SetBuiltInStyles({
    .mDisplay = YGDisplayFlex,
    .mFlexDirection = (orientation == Orientation::Horizontal)
      ? YGFlexDirectionRow
      : YGFlexDirectionColumn,
    .mFlexGrow = 1,
  });
  mThumb->OnDrag(std::bind_front(&ScrollBar::OnThumbDrag, this));

  // Hardcoded in XAML
  constexpr auto SmallPressedAnimation
    = LinearStyleTransition(std::chrono::milliseconds(16));

  using namespace PseudoClasses;
  static const Style sSmallChangeStyles {
    .mColor = ScrollBarButtonArrowForeground,
    .mFont = {
      ResolveGlyphFont(SystemFont::Body).WithSizeInPixels(ScrollBarButtonArrowIconFontSize),
      !important,
    },
    .mOpacity = 0,
    .mScaleX = { 1, SmallPressedAnimation },
    .mScaleY = { 1, SmallPressedAnimation },
    .mTranslateX = {0, SmallPressedAnimation},
    .mTranslateY = {0, SmallPressedAnimation},
    .mAnd = {
      { Hover, Style {
        .mColor = ScrollBarButtonArrowForegroundPointerOver,
      }},
      { Active, Style {
        .mColor = ScrollBarButtonArrowForegroundPressed,
        .mScaleX = (orientation == Orientation::Horizontal) ? 1.0 : ScrollBarButtonArrowScalePressed,
        .mScaleY = (orientation == Orientation::Vertical) ? 1.0 : ScrollBarButtonArrowScalePressed,
        .mTranslateX = (orientation == Orientation::Horizontal) ? 0 : 0.5f,
        .mTranslateY = (orientation == Orientation::Vertical) ? 0 : 1.0f,
      }},
    },
  };

  mSmallDecrement->SetBuiltInStyles({sSmallChangeStyles});
  mSmallIncrement->SetBuiltInStyles({sSmallChangeStyles});

  static const Style sLargeChangeStyles {
    .mFlexGrow = 1,
  };
  mLargeDecrement->SetBuiltInStyles({sLargeChangeStyles});
  mLargeIncrement->SetBuiltInStyles({sLargeChangeStyles});

  const bool isHorizontal = (orientation == Orientation::Horizontal);
  Style thumbStyles {
    .mBackgroundColor = ScrollBarThumbFill,
    .mBorderRadius = ScrollBarCornerRadius,
    .mHeight = {
      static_cast<float>(isHorizontal ? ScrollBarHorizontalThumbMinHeight : ScrollBarVerticalThumbMinHeight),
      ContractAnimation},
    .mWidth = {
      static_cast<float>(isHorizontal ? ScrollBarHorizontalThumbMinWidth : ScrollBarVerticalThumbMinWidth),
      ExpandAnimation},
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ScrollBarThumbFillDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ScrollBarThumbFillPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ScrollBarThumbFillPressed,
      }},
    },
  };
  switch (orientation) {
    case Orientation::Vertical:
      thumbStyles += Style {
        .mWidth = {ScrollBarHorizontalThumbMinWidth, ContractAnimation},
      };
      mSmallDecrement->SetText(upGlyph);
      mSmallIncrement->SetText(downGlyph);
      break;
    case Orientation::Horizontal:
      thumbStyles += Style {
        .mHeight = {ScrollBarHorizontalThumbMinHeight, ContractAnimation},
      };
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

  const Style smallChangeStyles {
    .mOpacity = {
      (hovered ? 1.0f : 0.0f),
      (hovered ? ExpandAnimation : ContractAnimation),
    },
  };
  mSmallDecrement->SetExplicitStyles(smallChangeStyles);
  mSmallIncrement->SetExplicitStyles(smallChangeStyles);

  if (mOrientation == Orientation::Horizontal) {
    mThumb->SetExplicitStyles({
      .mHeight = {
           static_cast<float>(hovered ? ScrollBarSize
           : ScrollBarHorizontalThumbMinHeight), hovered ? ExpandAnimation : ContractAnimation,
      },
    });
  }

  return Widget::OnComputedStyleChange(style, state);
}

Style ScrollBar::GetBuiltinStylesForOrientation() const {
  using namespace PseudoClasses;
  static const Style sBaseStyles {
    .mBackgroundColor = ScrollBarBackground,
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ScrollBarBackgroundDisabled,
      }},
      { Hover, Style {
        .mBackgroundColor = ScrollBarBackgroundPointerOver,
      }},
    },
  };
  static const Style sHorizontalStyles
    = sBaseStyles + Style {.mFlexDirection = YGFlexDirectionRow};
  static const Style sVerticalStyles
    = sBaseStyles + Style {.mFlexDirection = YGFlexDirectionColumn};

  if (mOrientation == Orientation::Horizontal) {
    return {sHorizontalStyles};
  } else {
    return {sVerticalStyles};
  }
}

void ScrollBar::ScrollBarButtonTick(ButtonTickKind kind) {
  OutputDebugStringA(
    std::format("Scroll bar button tick: {}\n", std::to_underlying(kind))
      .c_str());
}
void ScrollBar::UpdateLayout() {
  mLargeDecrement->SetExplicitStyles({
    .mFlexGrow = mValue - mMinimum,
  });
  mThumb->SetExplicitStyles({
    .mFlexGrow = mThumbSize,
  });
  mLargeIncrement->SetExplicitStyles({
    .mFlexGrow = mMaximum - mValue,
  });
}

void ScrollBar::OnThumbDrag(SkPoint* deltaXY) {
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
    = (mOrientation == Orientation::Horizontal) ? &SkPoint::fX : &SkPoint::fY;
  const auto deltaPixels = std::invoke(deltaPtr, deltaXY);
  const auto deltaV = deltaPixels / rangePixels;

  const auto rawValue = mValue + deltaV;
  const auto clampedValue = std::clamp(rawValue, mMinimum, mMaximum);

  if (rawValue != clampedValue) {
    const auto fixPx = (clampedValue - rawValue) * rangePixels;
    std::invoke(deltaPtr, deltaXY) += fixPx;
  }

  this->SetValue(std::clamp(clampedValue, mMinimum, mMaximum));
}

void ScrollBar::SetMinimum(float value) {
  mMinimum = value;
  this->UpdateLayout();
}

float ScrollBar::GetMinimum() const {
  return mMinimum;
}

void ScrollBar::SetMaximum(float value) {
  mMaximum = value;
  this->UpdateLayout();
}

float ScrollBar::GetValue() const {
  return mValue;
}

void ScrollBar::SetValue(float value) {
  mValue = value;
  this->UpdateLayout();
}

float ScrollBar::GetThumbSize() const {
  return mThumbSize;
}

void ScrollBar::SetThumbSize(float value) {
  mThumbSize = value;
  this->UpdateLayout();
}

}// namespace FredEmmott::GUI::Widgets