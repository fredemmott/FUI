// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBoxStyles.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/StyleClass.hpp>
#include <FredEmmott/GUI/StyleSheet.hpp>

namespace FredEmmott::GUI {

StyleClass ComboBoxItemStyleClass() noexcept {
  static const StyleClass ret = StyleClass::Make("ComboBoxItem");
  return ret;
}

StyleClass ComboBoxItemPillStyleClass() noexcept {
  static const StyleClass ret = StyleClass::Make("ComboBoxItemPill");
  return ret;
}

StyleSheet ComboBoxItemStyles() noexcept {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  const auto klass = ComboBoxItemStyleClass();
  static const StyleSheet ret {
    {
      klass,
      Style {
        .mBackgroundColor = ComboBoxItemBackground,
        .mBorderColor = ComboBoxItemBorderBrush,
        .mBorderRadius = ComboBoxItemCornerRadius,
        .mColor = ComboBoxItemForeground,
        .mMarginBottom = 2,
        .mMarginLeft = 5,
        .mMarginRight = 5,
        .mMarginTop = 2,
        .mPaddingBottom = ComboBoxItemThemePaddingBottom,
        .mPaddingRight = ComboBoxItemThemePaddingRight,
        .mPaddingTop = ComboBoxItemThemePaddingTop,
      },
    },
    {
      klass & PseudoClasses::Disabled,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundDisabled,
        .mBorderColor = ComboBoxItemBorderBrushDisabled,
        .mColor = ComboBoxItemForegroundDisabled,
      },
    },
    {
      klass & PseudoClasses::Hover,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundPointerOver,
        .mBorderColor = ComboBoxItemBorderBrushPointerOver,
        .mColor = ComboBoxItemForegroundPointerOver,
      },
    },
    {
      klass & PseudoClasses::Active,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundPressed,
        .mBorderColor = ComboBoxItemBorderBrushPressed,
        .mColor = ComboBoxItemForegroundPressed,
      },
    },
    {
      klass & PseudoClasses::Checked,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundSelected,
        .mBorderColor = ComboBoxItemBorderBrushSelected,
        .mColor = ComboBoxItemForegroundSelected,
      },
    },
    {
      klass & PseudoClasses::Checked & PseudoClasses::Disabled,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundSelectedDisabled,
        .mBorderColor = ComboBoxItemBorderBrushSelectedDisabled,
        .mColor = ComboBoxItemForegroundSelectedDisabled,
      },
    },
    {
      klass & PseudoClasses::Checked & PseudoClasses::Hover,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundSelectedPointerOver,
        .mBorderColor = ComboBoxItemBorderBrushSelectedPointerOver,
        .mColor = ComboBoxItemForegroundSelectedPointerOver,
      },
    },
    {
      klass & PseudoClasses::Checked & PseudoClasses::Active,
      Style {
        .mBackgroundColor = ComboBoxItemBackgroundSelectedPressed,
        .mBorderColor = ComboBoxItemBorderBrushSelectedPressed,
        .mColor = ComboBoxItemForegroundSelectedPressed,
      },
    },
  };
  return ret;
}

StyleSheet ComboBoxItemPillStyles() noexcept {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  const auto itemClass = ComboBoxItemStyleClass();
  const auto pillClass = ComboBoxItemPillStyleClass();
  const auto PillHeightAnimation = CubicBezierStyleTransition(
    ComboBoxItemScaleAnimationDuration, ControlFastOutSlowInKeySpline);
  static const StyleSheet ret {
    {
      pillClass,
      Style {
        .mBackgroundColor = ComboBoxItemPillFillBrush,
        .mBorderRadius = ComboBoxItemPillCornerRadius,
        .mHeight = {0, PillHeightAnimation},
        .mMarginLeft = 0.5,
        .mMarginRight = 6,
        .mMarginTop = 2.5,
        .mTop = {0, PillHeightAnimation},
        .mWidth = ComboBoxItemPillWidth,
      },
    },
    {
      ((itemClass & PseudoClasses::Checked), pillClass),
      Style {
        .mHeight = ComboBoxItemPillHeight,
      },
    },
    {
      ((itemClass & PseudoClasses::Checked),
       (pillClass & PseudoClasses::Active)),
      Style {
        .mHeight = ComboBoxItemPillHeight * ComboBoxItemPillMinScale,
        .mTop = (ComboBoxItemPillHeight
                 - (ComboBoxItemPillHeight * ComboBoxItemPillMinScale))
          / 2,
      },
    },
  };
  return ret;
}

StyleClass ComboBoxButtonStyleClass() noexcept {
  static const StyleClass ret = StyleClass::Make("ComboBoxButton");
  return ret;
}

StyleSheet ComboBoxButtonStyles() noexcept {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  const auto klass = ComboBoxButtonStyleClass();
  static const StyleSheet ret {
    {
      klass,
      Style {
        .mAlignSelf = YGAlignFlexStart,
        .mBackgroundColor = ComboBoxBackground,
        .mBorderColor = ComboBoxBorderBrush,
        .mBorderRadius = ControlCornerRadius,
        .mBorderWidth = ComboBoxBorderThemeThickness,
        .mColor = ComboBoxForeground,
        .mFlexDirection = YGFlexDirectionRow,
        .mFont = WidgetFont::ControlContent,
        .mMinWidth = ComboBoxThemeMinWidth,
        .mPaddingBottom = ComboBoxPaddingBottom,
        .mPaddingLeft = ComboBoxPaddingLeft,
        .mPaddingRight = ComboBoxPaddingRight,
        .mPaddingTop = ComboBoxPaddingTop,
      },
    },
    {
      klass & PseudoClasses::Disabled,
      Style {
        .mBackgroundColor = ComboBoxBackgroundDisabled,
        .mBorderColor = ComboBoxBorderBrushDisabled,
        .mColor = ComboBoxForegroundDisabled,
      },
    },
    {
      klass & PseudoClasses::Hover,
      Style {
        .mBackgroundColor = ComboBoxBackgroundPointerOver,
        .mBorderColor = ComboBoxBorderBrushPointerOver,
        .mColor = ComboBoxForegroundPointerOver,
      },
    },
    {
      klass & PseudoClasses::Active,
      Style {
        .mBackgroundColor = ComboBoxBackgroundPressed,
        .mBorderColor = ComboBoxBorderBrushPressed,
        .mColor = ComboBoxForegroundPressed,
      },
    },
  };
  return ret;
}
}// namespace FredEmmott::GUI
