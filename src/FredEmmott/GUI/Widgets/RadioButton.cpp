// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "RadioButton.hpp"

#include <FredEmmott/GUI/StaticTheme/RadioButton.hpp>

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {
namespace {
constexpr LiteralStyleClass RadioButtonStyleClass {"RadioButton"};
}// namespace

RadioButton::RadioButton(std::size_t id) : Widget(id, {RadioButtonStyleClass}) {
  this->ChangeDirectChildren([this] {
    mOuter = std::make_unique<Widget>(0);
    mFosterParent = std::make_unique<Widget>(1);
  });
  mOuter->SetChildren({mInner = new Widget(0)});

  this->BuiltInStyles() += Style().AlignContent(YGAlignCenter).Height(32);
  static const auto FosterParentStyles
    = Style()
        .AlignItems(YGAlignCenter)
        .PaddingLeft(8)
        .PaddingTop(6)
        .TranslateY(-2);
  mFosterParent->BuiltInStyles() = FosterParentStyles;
  this->InitializeOuterStyles();
  this->InitializeInnerStyles();
}

RadioButton::~RadioButton() = default;

Widget* RadioButton::GetFosterParent() const noexcept {
  return mFosterParent.get();
}

WidgetList RadioButton::GetDirectChildren() const noexcept {
  return {
    mOuter.get(),
    mFosterParent.get(),
  };
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

void RadioButton::InitializeOuterStyles() {
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  static const Style BaseStyle
    = Style()
        .AlignContent(YGAlignCenter)
        .AlignItems(YGAlignCenter)
        .AlignSelf(YGAlignCenter)
        .BorderRadius(10)
        .BorderWidth(RadioButtonBorderThemeThickness)
        .Height(20)
        .JustifyContent(YGJustifyCenter)
        .Width(20);

  YGNodeStyleSetBoxSizing(mOuter->GetLayoutNode(), YGBoxSizingContentBox);

  static const Style UncheckedStyle
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

  static const Style CheckedStyle
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

  mOuter->BuiltInStyles() = BaseStyle
    + Style().And(Checked, CheckedStyle).And(!Checked, UncheckedStyle);
}

void RadioButton::InitializeInnerStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  constexpr auto SizeAnimation = CubicBezierStyleTransition(
    ControlNormalAnimationDuration, ControlFastOutSlowInKeySpline);
  static const Style GlyphStyle
    = Style()
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
        .And(!Checked, Style().Display(YGDisplayNone));
  mInner->BuiltInStyles() = GlyphStyle;
}
}// namespace FredEmmott::GUI::Widgets