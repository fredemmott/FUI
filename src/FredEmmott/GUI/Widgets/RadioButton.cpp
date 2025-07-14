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

  this->SetBuiltInStyles(Style().AlignContent(YGAlignCenter).Height(32));
  mFosterParent->SetBuiltInStyles(
    Style()
      .AlignItems(YGAlignCenter)
      .PaddingLeft(8)
      .PaddingTop(6)
      .TranslateY(-2));
  this->InitializeInnerStyles();
  this->SetStyles();
}

RadioButton::~RadioButton() = default;

void RadioButton::SetIsChecked(const bool value) noexcept {
  if (mIsChecked == value) {
    return;
  }
  mIsChecked = value;
  this->SetStyles();
}

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
void RadioButton::SetStyles() {
  this->SetOuterStyles();
  this->SetInnerStyles();
}

void RadioButton::SetOuterStyles() {
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  static const Style BaseStyle = Style()
                                   .AlignContent(YGAlignCenter)
                                   .AlignItems(YGAlignCenter)
                                   .AlignSelf(YGAlignCenter)
                                   .BorderRadius(10)
                                   .BorderWidth(RadioButtonBorderThemeThickness)
                                   .Height(20)
                                   .JustifyContent(YGJustifyCenter)
                                   .Width(20);

  YGNodeStyleSetBoxSizing(mOuter->GetLayoutNode(), YGBoxSizingContentBox);

  static const Style UncheckedStyle = BaseStyle
    + Style()
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

  static const Style CheckedStyle = BaseStyle
    + Style()
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

  mOuter->SetBuiltInStyles(mIsChecked ? CheckedStyle : UncheckedStyle);
}

void RadioButton::SetInnerStyles() {
  mInner->ReplaceExplicitStyles(
    Style().Display(mIsChecked ? YGDisplayFlex : YGDisplayNone));
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
            .Width(14));
  mInner->SetBuiltInStyles(GlyphStyle);
}
}// namespace FredEmmott::GUI::Widgets