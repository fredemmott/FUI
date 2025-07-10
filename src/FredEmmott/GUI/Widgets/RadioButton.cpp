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

  this->InitializeInnerStyles();
  this->SetStyles();
}

RadioButton::~RadioButton() = default;

void RadioButton::SetIsChecked(const bool value) noexcept {
  if (mIsChecked == value) {
    return;
  }
  mIsChecked = value;
  mClicked = mClicked || value;
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
  return EventHandlerResult::StopPropagation;
}
void RadioButton::SetStyles() {
  this->SetOuterStyles();
  this->SetInnerStyles();
}

void RadioButton::SetOuterStyles() {
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  static const Style BaseStyle {
    .mAlignContent = YGAlignCenter,
    .mAlignItems = YGAlignCenter,
    .mAlignSelf = YGAlignCenter,
    .mBorderRadius = 10,
    .mBorderWidth = RadioButtonBorderThemeThickness,
    .mHeight = 20,
    .mJustifyContent = YGJustifyCenter,
    .mWidth = 20,
  };
  YGNodeStyleSetBoxSizing(mOuter->GetLayoutNode(), YGBoxSizingContentBox);
  static const Style UncheckedStyle = BaseStyle + Style {
    .mBackgroundColor = RadioButtonOuterEllipseFill,
    .mBorderColor = RadioButtonOuterEllipseStroke,
    .mAnd = {
      { Hover, Style {
        .mBackgroundColor = RadioButtonOuterEllipseFillPointerOver,
        .mBorderColor = RadioButtonOuterEllipseStrokePointerOver,
      }},
      { Active , Style {
        .mBackgroundColor = RadioButtonOuterEllipseFillPressed,
        .mBorderColor = RadioButtonOuterEllipseStrokePressed,
      }},
      { Disabled, Style {
        .mBackgroundColor = RadioButtonOuterEllipseFillDisabled,
        .mBorderColor = RadioButtonOuterEllipseStrokeDisabled,
      }}
    },
  };
  static const Style CheckedStyle = BaseStyle + Style {
    .mBackgroundColor = RadioButtonOuterEllipseCheckedFill,
    .mBorderColor = RadioButtonOuterEllipseCheckedStroke,
    .mAnd = {
      { Hover, Style {
        .mBackgroundColor = RadioButtonOuterEllipseCheckedFillPointerOver,
        .mBorderColor = RadioButtonOuterEllipseCheckedStrokePointerOver,
      }},
      { Active , Style {
        .mBackgroundColor = RadioButtonOuterEllipseCheckedFillPressed,
        .mBorderColor = RadioButtonOuterEllipseCheckedStrokePressed,
      }},
      { Disabled, Style {
        .mBackgroundColor = RadioButtonOuterEllipseCheckedFillDisabled,
        .mBorderColor = RadioButtonOuterEllipseCheckedStrokeDisabled,
      }}
    },
  };
  mOuter->SetBuiltInStyles(mIsChecked ? CheckedStyle : UncheckedStyle);
}

void RadioButton::SetInnerStyles() {
  mInner->ReplaceExplicitStyles(
    {.mDisplay = (mIsChecked ? YGDisplayFlex : YGDisplayNone)});
}

void RadioButton::InitializeInnerStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::RadioButton;
  using namespace PseudoClasses;
  constexpr auto SizeAnimation = CubicBezierStyleTransition(
    ControlNormalAnimationDuration, ControlFastOutSlowInKeySpline);
  static const Style GlyphStyle {
    .mAlignContent = YGAlignCenter,
    .mAlignItems = YGAlignCenter,
    .mAlignSelf = YGAlignCenter,
    .mBackgroundColor = RadioButtonCheckGlyphFill,
    .mBorderColor = RadioButtonCheckGlyphStroke,
    .mBorderRadius = 7,
    .mHeight = {RadioButtonCheckGlyphSize, SizeAnimation},
    .mWidth = {RadioButtonCheckGlyphSize, SizeAnimation},
    .mAnd = {
      {Hover, Style {
        .mBackgroundColor = RadioButtonCheckGlyphFillPointerOver,
        .mBorderColor = RadioButtonCheckGlyphStrokePointerOver,
        .mHeight = 14,
        .mWidth = 14,
      }},
      {Active, Style {
        .mBackgroundColor = RadioButtonCheckGlyphFillPressed,
        .mBorderColor = RadioButtonCheckGlyphStrokePressed,
        .mBorderRadius = 6,
        .mHeight = 10,
        .mWidth = 10,
      }},
      {Disabled, Style {
        .mBackgroundColor = RadioButtonCheckGlyphFillDisabled,
        .mBorderColor = RadioButtonCheckGlyphStrokeDisabled,
        .mHeight = 14,
        .mWidth = 14,
      }},
    },
  };
  mInner->SetBuiltInStyles(GlyphStyle);
}

}// namespace FredEmmott::GUI::Widgets