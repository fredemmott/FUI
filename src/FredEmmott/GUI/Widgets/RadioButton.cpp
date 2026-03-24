// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "RadioButton.hpp"

#include <FredEmmott/GUI/StaticTheme/RadioButton.hpp>

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {
namespace {
constexpr LiteralStyleClass RadioButtonStyleClass {"RadioButton"};

auto& RadioButtonStyles() {
  static const ImmutableStyle ret {
    Style()
      .AlignContent(Align::Center)
      .Height(32)
      .OutlineLeftOffset(7)
      .OutlineTopOffset(3)
      .OutlineRightOffset(7)
      .OutlineBottomOffset(3),
  };
  return ret;
}

auto& FosterParentStyles() {
  static const ImmutableStyle ret {
    Style()
      .AlignItems(Align::Center)
      .PaddingLeft(8)
      .PaddingTop(6)
      .TranslateY(-2),
  };
  return ret;
}

auto MakeOuterStyles() {
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  const Style BaseStyle
    = Style()
        .AlignContent(Align::Center)
        .AlignItems(Align::Center)
        .AlignSelf(Align::Center)
        .BorderRadius(10)
        .BorderWidth(RadioButtonBorderThemeThickness)
        .BoxSizing(BoxSizing::BorderBox)
        .Height(20)
        .JustifyContent(YGJustifyCenter)
        .Width(20);

  const Style UncheckedStyle
    = Style()
        .BackgroundColor(RadioButtonOuterEllipseFill)
        .BorderColor(RadioButtonOuterEllipseStroke)
        .And(
          Hover,
          Style()
            .BackgroundColor(RadioButtonOuterEllipseFillPointerOver)
            .BorderColor(RadioButtonOuterEllipseStrokePointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(RadioButtonOuterEllipseFillPressed)
            .BorderColor(RadioButtonOuterEllipseStrokePressed))
        .And(
          Disabled,
          Style()
            .BackgroundColor(RadioButtonOuterEllipseFillDisabled)
            .BorderColor(RadioButtonOuterEllipseStrokeDisabled));

  const Style CheckedStyle
    = Style()
        .BackgroundColor(RadioButtonOuterEllipseCheckedFill)
        .BorderColor(RadioButtonOuterEllipseCheckedStroke)
        .And(
          Hover,
          Style()
            .BackgroundColor(RadioButtonOuterEllipseCheckedFillPointerOver)
            .BorderColor(RadioButtonOuterEllipseCheckedStrokePointerOver))
        .And(
          Active,
          Style()
            .BackgroundColor(RadioButtonOuterEllipseCheckedFillPressed)
            .BorderColor(RadioButtonOuterEllipseCheckedStrokePressed))
        .And(
          Disabled,
          Style()
            .BackgroundColor(RadioButtonOuterEllipseCheckedFillDisabled)
            .BorderColor(RadioButtonOuterEllipseCheckedStrokeDisabled));

  return BaseStyle
    + Style().And(Checked, CheckedStyle).And(!Checked, UncheckedStyle);
}

auto& OuterStyles() {
  static const ImmutableStyle ret {MakeOuterStyles()};
  return ret;
}

auto& InnerStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  constexpr auto SizeAnimation = CubicBezierStyleTransition(
    ControlNormalAnimationDuration, ControlFastOutSlowInKeySpline);

  // We scale the thumb as that only affects visuals, not layouts - in
  // particular, it doesn't propagate the dirty flag to all ancestor nodes
  static constexpr auto HoverScale = 14.f / RadioButtonCheckGlyphSize;
  static constexpr auto ActiveScale = 10.f / RadioButtonCheckGlyphSize;
  static constexpr auto DisabledScale = HoverScale;

  static const ImmutableStyle ret {
    Style()
      .AlignContent(Align::Center)
      .AlignItems(Align::Center)
      .AlignSelf(Align::Center)
      .BackgroundColor(RadioButtonCheckGlyphFill)
      .BorderColor(RadioButtonCheckGlyphStroke)
      .BorderRadius(7)
      .Display(Display::None)
      .Height(RadioButtonCheckGlyphSize)
      .Width(RadioButtonCheckGlyphSize)
      .ScaleX(1, SizeAnimation)
      .ScaleY(1, SizeAnimation)
      .TransformOriginX(0.5f)
      .TransformOriginY(0.5f)
      .And(
        Hover,
        Style()
          .BackgroundColor(RadioButtonCheckGlyphFillPointerOver)
          .BorderColor(RadioButtonCheckGlyphStrokePointerOver)
          .ScaleX(HoverScale)
          .ScaleY(HoverScale))
      .And(
        Active,
        Style()
          .BackgroundColor(RadioButtonCheckGlyphFillPressed)
          .BorderColor(RadioButtonCheckGlyphStrokePressed)
          .BorderRadius(6)
          .Display(Display::Flex)
          .ScaleX(ActiveScale)
          .ScaleY(ActiveScale))
      .And(
        Disabled,
        Style()
          .BackgroundColor(RadioButtonCheckGlyphFillDisabled)
          .BorderColor(RadioButtonCheckGlyphStrokeDisabled)
          .ScaleX(DisabledScale)
          .ScaleY(DisabledScale))
      .And(Checked, Style().Display(Display::Flex)),
  };
  return ret;
}
}// namespace

RadioButton::RadioButton(Window* const window)
  : Widget(window, RadioButtonStyleClass, RadioButtonStyles()),
    ISelectionItem(this) {
  const auto inner = new Widget(
    window, LiteralStyleClass {"RadioButton/Inner"}, InnerStyles());
  const auto outer
    = (new Widget(
         window, LiteralStyleClass {"RadioButton/Outer"}, OuterStyles()))
        ->SetStructuralChildren({inner});
  this->SetStructuralParentForLogicalChildren(
    mFosterParent = new Widget(
      window,
      LiteralStyleClass {"RadioButton/FosterParent"},
      FosterParentStyles()));

  this->SetStructuralChildren({outer, mFosterParent});
}

RadioButton::~RadioButton() = default;

bool RadioButton::IsSelected() const noexcept {
  return IsChecked();
}

Widget::ComputedStyleFlags RadioButton::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  using enum ComputedStyleFlags;
  return Widget::OnComputedStyleChange(style, state) | InheritableActiveState
    | InheritableHoverState;
}

void RadioButton::Select() {
  if (IsChecked()) {
    return;
  }
  this->SetIsChecked(true);
  mWasSelected = true;

  for (auto&& sibling: GetSelectionContainer()->GetSelectionItems()) {
    if (sibling != this) {
      CastSelectionSibling<RadioButton>(sibling)->SetIsChecked(false);
    }
  }
}

Widget::EventHandlerResult RadioButton::OnClick(const MouseEvent&) {
  this->Select();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets