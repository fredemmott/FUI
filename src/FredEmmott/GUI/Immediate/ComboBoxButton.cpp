// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBoxButton.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "Button.hpp"
#include "FontIcon.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

Result<&EndComboBoxButton, void> BeginComboBoxButton(
  bool* clicked,
  const ID id) {
  using Button = Widgets::Button;
  using Widget = Widgets::Widget;
  const auto button = BeginWidget<Button>(id);
  if (clicked) {
    *clicked = button->mClicked.TestAndClear();
  }

  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  using namespace PseudoClasses;
  static const Style styles {
    .mAlignSelf = YGAlignFlexStart,
    .mBackgroundColor = ComboBoxBackground,
    .mBorderColor =ComboBoxBorderBrush,
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
    .mAnd = {
      { Disabled, Style {
          .mBackgroundColor = ComboBoxBackgroundDisabled,
          .mBorderColor = ComboBoxBorderBrushDisabled,
          .mColor = ComboBoxForegroundDisabled,
      }},
      { Hover, Style {
          .mBackgroundColor = ComboBoxBackgroundPointerOver,
          .mBorderColor = ComboBoxBorderBrushPointerOver,
          .mColor = ComboBoxForegroundPointerOver,
      }},
      { Active, Style {
          .mBackgroundColor = ComboBoxBackgroundPressed,
          .mBorderColor = ComboBoxBorderBrushPressed,
          .mColor = ComboBoxForegroundPressed,
      }},
    },
  };
  button->SetBuiltInStyles({styles});

  BeginWidget<Widget>(ID {"container"});
  GetCurrentParentNode()->SetAdditionalBuiltInStyles({
    .mFlexGrow = 1,
  });
  button->SetContextIfUnset([]() {
    return std::make_unique<PopupAnchorContext>(GetCurrentParentNode());
  });
  BeginWidget<Widget>(ID {0});
  GetCurrentParentNode()->SetBuiltInStyles({
    .mDisplay = YGDisplayContents,
  });
  return {button};
};

void EndComboBoxButton() {
  using Button = Widgets::Button;
  EndWidget<Widget>();// inner
  EndWidget<Widget>();// container

  using namespace StaticTheme::Common;
  using namespace PseudoClasses;
  constexpr auto KeySpline = ControlFastOutSlowInKeySpline;
  constexpr auto FasterAnimation
    = CubicBezierStyleTransition(ControlFasterAnimationDuration, KeySpline);

  FontIcon("\ue70d", FontIconSize::Body, ID {"glyph"});// ChevronDown
  // MarginRight of 14 is in the Xaml without an alias; MarginLeft
  // is not set in the XAML at all.
  GetCurrentNode()->ReplaceExplicitStyles({
    .mAlignSelf = YGAlignFlexEnd,
    .mFlexGrow = 0,
    .mMarginLeft = 14,
    .mMarginRight = 14,
    .mTop = {
      0, FasterAnimation,
    },
    .mAnd = {
      { Active, Style {
        .mTop = 2,
      }},
    },
  });

  EndWidget<Button>();
}

Result<nullptr, bool> ComboBoxButton(
  const std::string_view label,
  const ID id) {
  bool clicked {};
  const auto button = BeginComboBoxButton(&clicked, id);
  Label(label, ID {0});
  EndComboBoxButton();
  return {button, clicked};
}

}// namespace FredEmmott::GUI::Immediate