// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBar.hpp"

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>

#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme::ScrollBar;

namespace {

const auto ScrollBarStyleClass = Style::Class::Make("ScrollBar");
const auto VerticalScrollBarStyleClass
  = Style::Class::Make("VerticalScrollBar");
const auto HorizontalScrollBarStyleClass
  = Style::Class::Make("HorizontalScrollBar");
const auto SmallDecrementStyleClass
  = Style::Class::Make("ScrollBarSmallDecrement");
const auto LargeDecrementStyleClass
  = Style::Class::Make("ScrollBarLargeDecrement");
const auto ThumbStyleClass = Style::Class::Make("ScrollBarThumb");
const auto LargeIncrementStyleClass
  = Style::Class::Make("ScrollBarLargeIncrement");
const auto SmallIncrementStyleClass
  = Style::Class::Make("ScrollBarSmallIncrement");

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
  : Widget(
      id,
      {ScrollBarStyleClass,
       (orientation == Orientation::Horizontal ? HorizontalScrollBarStyleClass
                                               : VerticalScrollBarStyleClass)}),
    mOrientation(orientation),
    mBuiltinStyles(GetBuiltinStylesForOrientation()) {
  // https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  constexpr auto leftGlyph = "\uedd9";
  constexpr auto rightGlyph = "\uedda";
  constexpr auto upGlyph = "\ueddb";
  constexpr auto downGlyph = "\ueddc";

  this->ChangeDirectChildren([this] {
    mSmallDecrement.reset(new Label(0, {SmallDecrementStyleClass}));
    mLargeDecrement.reset(new Widget(0, {LargeDecrementStyleClass}));
    mThumb.reset(new Widget(0, {ThumbStyleClass}));
    mLargeIncrement.reset(new Widget(0, {LargeIncrementStyleClass}));
    mSmallIncrement.reset(new Label(0, {SmallIncrementStyleClass}));
  });

  // Hardcoded in XAML
  const auto SmallPressedAnimation
    = LinearStyleTransition(std::chrono::milliseconds(16));

  static const WidgetStyles sSmallChangeStyles {
    .mBase = {
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
    },
    .mHover = {
      .mColor = ScrollBarButtonArrowForegroundPointerOver,
    },
    .mActive = {
      .mColor = ScrollBarButtonArrowForegroundPressed,
      .mScaleX = (orientation == Orientation::Horizontal) ? 1.0 : ScrollBarButtonArrowScalePressed,
      .mScaleY = (orientation == Orientation::Vertical) ? 1.0 : ScrollBarButtonArrowScalePressed,
      .mTranslateX = (orientation == Orientation::Horizontal) ? 0 : 0.5f,
      .mTranslateY = (orientation == Orientation::Vertical) ? 0 : 1.0f,
    },
  };

  mSmallDecrement->SetBuiltInStyles(sSmallChangeStyles);
  mSmallIncrement->SetBuiltInStyles(sSmallChangeStyles);

  static const WidgetStyles sLargeChangeStyles {
    .mBase = {
      .mFlexGrow = 1,
    },
  };
  mLargeDecrement->SetBuiltInStyles(sLargeChangeStyles);
  mLargeIncrement->SetBuiltInStyles(sLargeChangeStyles);

  const bool isHorizontal = (orientation == Orientation::Horizontal);

  auto thumbStyles  = WidgetStyles {
    .mBase = Style {
      .mBackgroundColor = ScrollBarThumbFill,
      .mBorderRadius = ScrollBarCornerRadius,
      .mHeight = {
        static_cast<float>(isHorizontal ? ScrollBarHorizontalThumbMinHeight : ScrollBarVerticalThumbMinHeight),
        ContractAnimation},
      .mWidth = {
        static_cast<float>(isHorizontal ? ScrollBarHorizontalThumbMinWidth : ScrollBarVerticalThumbMinWidth),
        ExpandAnimation},
    },
    .mDisabled = Style {
      .mBackgroundColor = ScrollBarThumbFillDisabled,
    },
    .mHover = Style {
      .mBackgroundColor = ScrollBarThumbFillPointerOver,
    },
    .mActive = Style {
      .mBackgroundColor = ScrollBarThumbFillPressed,
    },
  };
  switch (orientation) {
    case Orientation::Vertical:
      thumbStyles.mBase += Style {
        .mWidth = {ScrollBarHorizontalThumbMinWidth, ContractAnimation},
      };
      mSmallDecrement->SetText(upGlyph);
      mSmallIncrement->SetText(downGlyph);
      break;
    case Orientation::Horizontal:
      thumbStyles.mBase += Style {
        .mHeight = {ScrollBarHorizontalThumbMinHeight, ContractAnimation},
      };
      mSmallDecrement->SetText(leftGlyph);
      mSmallIncrement->SetText(rightGlyph);
      break;
  }

  mThumb->SetBuiltInStyles(thumbStyles);
}

ScrollBar::~ScrollBar() = default;

WidgetList ScrollBar::GetDirectChildren() const noexcept {
  return {
    mSmallDecrement.get(),
    mLargeDecrement.get(),
    mThumb.get(),
    mLargeIncrement.get(),
    mSmallIncrement.get(),
  };
}
WidgetStyles ScrollBar::GetBuiltInStyles() const {
  return mBuiltinStyles;
}

Widget::ComputedStyleFlags ScrollBar::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  const bool hovered = (state & StateFlags::Hovered) == StateFlags::Hovered;

  const WidgetStyles smallChangeStyles {
    .mBase = {
      .mOpacity = {
        (hovered ? 1.0f : 0.0f),
        (hovered ? ExpandAnimation : ContractAnimation),
      },
    },
  };
  mSmallDecrement->SetExplicitStyles(smallChangeStyles);
  mSmallIncrement->SetExplicitStyles(smallChangeStyles);

  if (mOrientation == Orientation::Horizontal) {
    mThumb->SetExplicitStyles({
    .mBase = {
      .mHeight = {
           static_cast<float>(hovered ? ScrollBarSize
           : ScrollBarHorizontalThumbMinHeight), hovered ? ExpandAnimation : ContractAnimation,
        },
    },
  });
  }

  return Widget::OnComputedStyleChange(style, state);
}

WidgetStyles ScrollBar::GetBuiltinStylesForOrientation() const {
  static const WidgetStyles sBaseStyles {
    .mBase = {
      .mBackgroundColor = ScrollBarBackground,
    },
    .mDisabled = {
      .mBackgroundColor = ScrollBarBackgroundDisabled,
    },
    .mHover = {
      .mBackgroundColor = ScrollBarBackgroundPointerOver,
    },
  };
  static const WidgetStyles sHorizontalStyles = sBaseStyles + WidgetStyles {
    .mBase = {
      .mFlexDirection = YGFlexDirectionRow,
    },
  };
  static const WidgetStyles sVerticalStyles = sBaseStyles + WidgetStyles {
    .mBase = {
      .mFlexDirection = YGFlexDirectionColumn,
    },
  };

  if (mOrientation == Orientation::Horizontal) {
    return sHorizontalStyles;
  } else {
    return sVerticalStyles;
  }
}

}// namespace FredEmmott::GUI::Widgets