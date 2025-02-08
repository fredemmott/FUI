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

const auto ScrollBarStyleClass = StyleClass::Make("ScrollBar");
const auto VerticalScrollBarStyleClass = StyleClass::Make("VerticalScrollBar");
const auto HorizontalScrollBarStyleClass
  = StyleClass::Make("HorizontalScrollBar");
const auto SmallDecrementStyleClass
  = StyleClass::Make("ScrollBarSmallDecrement");
const auto LargeDecrementStyleClass
  = StyleClass::Make("ScrollBarLargeDecrement");
const auto ThumbStyleClass = StyleClass::Make("ScrollBarThumb");
const auto LargeIncrementStyleClass
  = StyleClass::Make("ScrollBarLargeIncrement");
const auto SmallIncrementStyleClass
  = StyleClass::Make("ScrollBarSmallIncrement");

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

  using enum Style::PseudoClass;
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
  using enum Style::PseudoClass;
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

}// namespace FredEmmott::GUI::Widgets