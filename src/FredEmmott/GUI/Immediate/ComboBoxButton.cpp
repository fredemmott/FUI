// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ComboBoxButton.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>

#include "Button.hpp"
#include "FontIcon.hpp"

namespace FredEmmott::GUI::Immediate {
using namespace immediate_detail;

namespace {
constexpr LiteralStyleClass ComboBoxButtonStyleClass {"ComboBoxButton"};

auto& ComboBoxButtonStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  using namespace PseudoClasses;
  static const ImmutableStyle ret {
    Style()
      .AlignSelf(Align::FlexStart)
      .BackgroundColor(ComboBoxBackground)
      .BorderColor(ComboBoxBorderBrush)
      .BorderRadius(ControlCornerRadius)
      .BorderWidth(ComboBoxBorderThemeThickness)
      .Color(ComboBoxForeground)
      .FlexDirection(FlexDirection::Row)
      .Font(WidgetFont::ControlContent)
      .MinHeight(ComboBoxMinHeight)
      .MinWidth(ComboBoxThemeMinWidth)
      .Padding(ComboBoxPadding)
      .And(
        Disabled,
        Style()
          .BackgroundColor(ComboBoxBackgroundDisabled)
          .BorderColor(ComboBoxBorderBrushDisabled)
          .Color(ComboBoxForegroundDisabled))
      .And(
        Hover,
        Style()
          .BackgroundColor(ComboBoxBackgroundPointerOver)
          .BorderColor(ComboBoxBorderBrushPointerOver)
          .Color(ComboBoxForegroundPointerOver))
      .And(
        Active,
        Style()
          .BackgroundColor(ComboBoxBackgroundPressed)
          .BorderColor(ComboBoxBorderBrushPressed)
          .Color(ComboBoxForegroundPressed)),
  };
  return ret;
}

}// namespace

ComboBoxButtonResult<&EndComboBoxButton, void> BeginComboBoxButton(
  bool* clicked,
  const ID id) {
  using Button = Widgets::Button;
  const auto button
    = BeginWidget<Button>(id, ComboBoxButtonStyleClass, ComboBoxButtonStyles());

  if (clicked) {
    *clicked = button->ConsumeWasActivated();
  }

  using namespace StaticTheme::Common;
  using namespace StaticTheme::ComboBox;
  using namespace PseudoClasses;

  static const ImmutableStyle OuterStyles {
    Style().AlignSelf(Align::Center).FlexGrow(1),
  };
  static const ImmutableStyle InnerStyles {
    Style().Display(Display::Contents),
  };
  BeginWidget<Widget>(
    ID {"container"}, LiteralStyleClass {"ComboBox/Button/Outer"}, OuterStyles);
  button->SetContextIfUnset<PopupAnchorContext>(GetCurrentParentNode());
  BeginWidget<Widget>(
    ID {0}, LiteralStyleClass {"ComboBox/Button/Inner"}, InnerStyles);
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
  GetCurrentNode()->SetMutableStyles(
    Style()
      .AlignSelf(Align::Center)
      .FlexGrow(0)
      .MarginLeft(14)
      .MarginRight(14)
      .TranslateY(0, FasterAnimation)
      .And(Active, Style().TranslateY(2)));

  EndWidget<Button>();
}

ComboBoxButtonResult<nullptr, bool> ComboBoxButton(
  const std::string_view label,
  const ID id) {
  bool clicked {};
  const auto button = BeginComboBoxButton(&clicked, id);
  Label(label, ID {0});
  EndComboBoxButton();
  return {button, clicked};
}

}// namespace FredEmmott::GUI::Immediate