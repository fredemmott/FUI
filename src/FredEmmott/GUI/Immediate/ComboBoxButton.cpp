// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBoxButton.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/detail/ComboBoxStyles.hpp>

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

  button->AddStyleClass(ComboBoxButtonStyleClass());
  button->ReplaceBuiltInStyleSheet(ComboBoxButtonStyles());

  BeginWidget<Widget>(ID {"container"});
  GetCurrentParentNode()->AppendBuiltInStyles({
    .mFlexGrow = 1,
  });
  BeginWidget<Widget>(ID {0});
  GetCurrentParentNode()->AppendBuiltInStyles({.mDisplay = YGDisplayContents});
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
  auto node = GetCurrentNode();
  node->AppendBuiltInStyleSheet({
    {
      StyleSelector { node },
      Style {
        .mAlignSelf = YGAlignFlexEnd,
        .mFlexGrow = 0,
        .mMarginLeft = 14,
        .mMarginRight = 14,
        .mTop = {
          -2, FasterAnimation,
          },
      },
    },
    {
      StyleSelector { node } & PseudoClasses::Active,
      Style {
        .mTop = 0,
      }
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