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
      .AlignContent(YGAlignCenter)
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
      .AlignItems(YGAlignCenter)
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
        .AlignContent(YGAlignCenter)
        .AlignItems(YGAlignCenter)
        .AlignSelf(YGAlignCenter)
        .BorderRadius(10)
        .BorderWidth(RadioButtonBorderThemeThickness)
        .BoxSizing(YGBoxSizingContentBox)
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
  static const ImmutableStyle ret {
    Style()
      .AlignContent(YGAlignCenter)
      .AlignItems(YGAlignCenter)
      .AlignSelf(YGAlignCenter)
      .BackgroundColor(RadioButtonCheckGlyphFill)
      .BorderColor(RadioButtonCheckGlyphStroke)
      .BorderRadius(7)
      .Height(RadioButtonCheckGlyphSize, SizeAnimation)
      .Width(RadioButtonCheckGlyphSize, SizeAnimation)
      .And(
        Hover,
        Style()
          .BackgroundColor(RadioButtonCheckGlyphFillPointerOver)
          .BorderColor(RadioButtonCheckGlyphStrokePointerOver)
          .Height(14)
          .Width(14))
      .And(
        Active,
        Style()
          .BackgroundColor(RadioButtonCheckGlyphFillPressed)
          .BorderColor(RadioButtonCheckGlyphStrokePressed)
          .BorderRadius(6)
          .Height(10)
          .Width(10))
      .And(
        Disabled,
        Style()
          .BackgroundColor(RadioButtonCheckGlyphFillDisabled)
          .BorderColor(RadioButtonCheckGlyphStrokeDisabled)
          .Height(14)
          .Width(14))
      .And(!Checked, Style().Display(YGDisplayNone)),
  };
  return ret;
}
}// namespace

RadioButton::RadioButton(std::size_t id)
  : Widget(id, RadioButtonStyles(), {RadioButtonStyleClass}) {
  const auto inner = new Widget({}, InnerStyles());
  const auto outer = (new Widget({}, OuterStyles()))->SetChildren({inner});
  mFosterParent = new Widget({}, FosterParentStyles());

  this->SetDirectChildren({outer, mFosterParent});
}

RadioButton::~RadioButton() = default;

Widget* RadioButton::GetFosterParent() const noexcept {
  return mFosterParent;
}

Widget::ComputedStyleFlags RadioButton::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  using enum ComputedStyleFlags;
  return Widget::OnComputedStyleChange(style, state) | InheritableActiveState
    | InheritableHoverState;
}

Widget::EventHandlerResult RadioButton::OnClick(const MouseEvent&) {
  this->SetIsChecked(true);
  mChanged = true;
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets