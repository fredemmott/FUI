// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBoxButton.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "Button.hpp"
#include "FontIcon.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

void BeginComboBoxButton(bool* clicked, const ID id) {
  using Button = Widgets::Button;
  using Widget = Widgets::Widget;
  BeginWidget<Button>(id);
  auto button = GetCurrentParentNode<Button>();
  if (clicked) {
    *clicked = button->mClicked.TestAndClear();
  }

  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  static const WidgetStyles styles {
    .mBase = {
      .mBackgroundColor = ComboBoxBackground,
      .mBorderColor =ComboBoxBorderBrush,
      .mBorderRadius = ControlCornerRadius,
      .mBorderWidth = ComboBoxBorderThemeThickness,
      .mColor = ComboBoxForeground,
      .mFlexBasis = ComboBoxThemeMinWidth,
      .mFlexDirection = YGFlexDirectionRow,
      .mFont = WidgetFont::ControlContent,
      .mPaddingBottom = ComboBoxPaddingBottom,
      .mPaddingLeft = ComboBoxPaddingLeft,
      .mPaddingRight = ComboBoxPaddingRight,
      .mPaddingTop = ComboBoxPaddingTop,
    },
    .mDisabled = {
      .mBackgroundColor = ComboBoxBackgroundDisabled,
      .mBorderColor = ComboBoxBorderBrushDisabled,
      .mColor = ComboBoxForegroundDisabled,
    },
    .mHover = {
      .mBackgroundColor = ComboBoxBackgroundPointerOver,
      .mBorderColor = ComboBoxBorderBrushPointerOver,
      .mColor = ComboBoxForegroundPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ComboBoxBackgroundPressed,
      .mBorderColor = ComboBoxBorderBrushPressed,
      .mColor = ComboBoxForegroundPressed,
    },
  };
  button->SetBuiltInStyles(styles);

  BeginWidget<Widget>(ID {"container"});
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({
    .mBase = {
      .mFlexGrow = 1,
    },
  });
  BeginWidget<Widget>(ID {0});
  GetCurrentParentNode()->SetBuiltInStyles({{.mDisplay = YGDisplayContents}});
};

void EndComboBoxButton() {
  using Button = Widgets::Button;
  EndWidget<Widget>();// inner
  EndWidget<Widget>();// container

  using namespace StaticTheme::Common;
  constexpr auto KeySpline = ControlFastOutSlowInKeySpline;
  constexpr auto FasterAnimation
    = CubicBezierStyleTransition(ControlFasterAnimationDuration, KeySpline);

  FontIcon("\ue70d", FontIconSize::Body, ID {"glyph"});// ChevronDown
  // This is hardcoded in the XAML :(
  GetCurrentNode()->SetExplicitStyles({
    .mBase = {
      .mAlignSelf = YGAlignFlexEnd,
      .mFlexGrow = 0,
      .mMarginRight = 14,
      .mMarginTop = -2,
      .mTop = {
        0, FasterAnimation,
      },
    },
    .mActive = {
      .mTop = 2,
    },
  });

  EndWidget<Button>();
}

bool ComboBoxButton(const std::string_view label, const ID id) {
  bool clicked {};
  BeginComboBoxButton(&clicked, id);
  Label(label, ID {0});
  EndComboBoxButton();
  return clicked;
}

}// namespace FredEmmott::GUI::Immediate