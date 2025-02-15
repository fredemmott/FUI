// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ScrollBarStyles.hpp"

#include <FredEmmott/GUI/StaticTheme/ScrollBar.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>

namespace FredEmmott::GUI {

StyleClass ScrollBarStyleClass() {
  static auto ret = StyleClass::Make("ScrollBar");
  return ret;
}

StyleClass ScrollBarHorizontalStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarHorizontal");
  return ret;
}

StyleClass ScrollBarVerticalStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarVertical");
  return ret;
}

StyleClass ScrollBarSmallChangeStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarSmallChange");
  return ret;
}

StyleClass ScrollBarSmallDecrementStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarSmallDecrement");
  return ret;
}

StyleClass ScrollBarSmallIncrementStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarSmallIncrement");
  return ret;
}

StyleClass ScrollBarThumbStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarThumb");
  return ret;
}

StyleClass ScrollBarLargeChangeStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarLargeChange");
  return ret;
}

StyleClass ScrollBarLargeDecrementStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarLargeDecrement");
  return ret;
}

StyleClass ScrollBarLargeIncrementStyleClass() {
  static auto ret = StyleClass::Make("ScrollBarLargeIncrement");
  return ret;
}

namespace {
using namespace StaticTheme::ScrollBar;

const auto ContractAnimation = CubicBezierStyleTransition(
  ScrollBarContractBeginTime,
  ScrollBarOpacityChangeDuration,
  StaticTheme::Common::ControlFastOutSlowInKeySpline);
const auto ExpandAnimation = CubicBezierStyleTransition(
  ScrollBarExpandBeginTime,
  ScrollBarOpacityChangeDuration,
  StaticTheme::Common::ControlFastOutSlowInKeySpline);
// Hardcoded in XAML
const auto SmallPressedAnimation
  = LinearStyleTransition(std::chrono::milliseconds(16));
}// namespace

StyleSheet ScrollBarSmallChangeStyles() {
  const auto scrollBar = ScrollBarStyleClass();
  const auto klass = ScrollBarSmallChangeStyleClass();
  static StyleSheet ret {
    {
      klass,
      Style {
        .mColor = ScrollBarButtonArrowForeground,
        .mFont = {
          ResolveGlyphFont(SystemFont::Body).WithSizeInPixels(ScrollBarButtonArrowIconFontSize),
          !important,
        },
        .mOpacity = { 0, ContractAnimation },
        .mScaleX = { 1, SmallPressedAnimation },
        .mScaleY = { 1, SmallPressedAnimation },
        .mTranslateX = {0, SmallPressedAnimation},
        .mTranslateY = {0, SmallPressedAnimation},
      },
  },
    {
      klass & PseudoClasses::Hover,
      Style {
        .mColor = ScrollBarButtonArrowForegroundPointerOver,
      },
    },
    {
      klass & PseudoClasses::Active,
      Style {
        .mColor = ScrollBarButtonArrowForegroundPointerOver,
      }
    },
    {
      (scrollBar & PseudoClasses::Hover, klass),
      Style {
        .mOpacity = { 1.0f, ExpandAnimation },
      },
    },
    {
      (ScrollBarHorizontalStyleClass(), klass & PseudoClasses::Active),
      Style {
        .mScaleY = ScrollBarButtonArrowScalePressed,
        .mTranslateY = 1.0f,
      },
    },
    {
      (ScrollBarVerticalStyleClass(), klass & PseudoClasses::Active),
      Style {
        .mScaleX = ScrollBarButtonArrowScalePressed,
        .mTranslateX = 0.5f,
      },
    },
  };
  return ret;
}

StyleSheet ScrollBarLargeChangeStyles() {
  static const StyleSheet ret {{
    ScrollBarLargeChangeStyleClass(),
    Style {.mFlexGrow = 1.0},
  }};
  return ret;
}

StyleSheet ScrollBarThumbStyles() {
  const auto thumb = ScrollBarThumbStyleClass();
  static const StyleSheet ret {
    {
      thumb,
      Style {
        .mBackgroundColor = ScrollBarThumbFill,
        .mBorderRadius = ScrollBarCornerRadius,
      },
    },
    {
      thumb & PseudoClasses::Disabled,
      Style {
        .mBackgroundColor = ScrollBarThumbFillDisabled,
      },
    },
    {
      thumb & PseudoClasses::Hover,
      Style {
        .mBackgroundColor = ScrollBarThumbFillPointerOver,
      },
    },
    {
      thumb & PseudoClasses::Active,
      Style {
        .mBackgroundColor = ScrollBarThumbFillPressed,
      },
    },
    {
      (ScrollBarHorizontalStyleClass(), thumb),
      Style {
        .mHeight = {
          ScrollBarHorizontalThumbMinHeight,
          ContractAnimation,
        },
        .mWidth = ScrollBarHorizontalThumbMinWidth,
      },
    },
    {
      (ScrollBarHorizontalStyleClass() & PseudoClasses::Hover, thumb),
      Style {
        .mHeight = {
          ScrollBarSize,
          ExpandAnimation,
        },
        .mWidth = ScrollBarHorizontalThumbMinWidth,
      },
    },
    {
      (ScrollBarVerticalStyleClass(), thumb),
      Style {
        .mHeight = ScrollBarVerticalThumbMinHeight,
        .mWidth = {ScrollBarVerticalThumbMinWidth, ContractAnimation},
      },
    },
  };
  return ret;
}

StyleSheet ScrollBarStyles() {
  const auto scrollBar = ScrollBarStyleClass();
  static const StyleSheet ret {
    {scrollBar,
     Style {
       .mBackgroundColor = ScrollBarBackground,
     }},
    {
      scrollBar & PseudoClasses::Disabled,
      Style {
        .mBackgroundColor = ScrollBarBackgroundDisabled,
      },
    },
    {
      scrollBar & PseudoClasses::Hover,
      Style {
        .mBackgroundColor = ScrollBarBackgroundPointerOver,
      },
    },
    {
      scrollBar & ScrollBarHorizontalStyleClass(),
      Style {
        .mFlexDirection = YGFlexDirectionRow,
      },
    },
    {
      scrollBar & ScrollBarVerticalStyleClass(),
      Style {
        .mFlexDirection = YGFlexDirectionColumn,
      },
    },
  };
  return ret;
}

}// namespace FredEmmott::GUI