// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "AppBarButton.hpp"

#include "FredEmmott/GUI/StaticTheme/AppBarButton.hpp"
#include "Label.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass AppBarButtonStyleClass {"AppBarButton"};
constexpr LiteralStyleClass AppBarButtonOutlineStyleClass {
  "AppBarButton/Outline"};
constexpr LiteralStyleClass AppBarButtonCompactStyleClass {
  "AppBarButton/Compact"};
constexpr LiteralStyleClass AppBarButtonLabelRightStyleClass {
  "AppBarButton/LabelRight"};
constexpr LiteralStyleClass AppBarButtonLabelCollapsedStyleClass {
  "AppBarButton/LabelCollapsed"};

constexpr LiteralStyleClass AppBarButtonBorderContainerStyleClass {
  "AppBarButton/Border"};
constexpr LiteralStyleClass AppBarButtonContentContainerStyleClass {
  "AppBarButton/Content"};
constexpr LiteralStyleClass AppBarButtonGlyphStyleClass {"AppBarButton/Glyph"};
constexpr LiteralStyleClass AppBarButtonLabelStyleClass {"AppBarButton/Label"};

const auto& AppBarButtonStyle() {
  static const ImmutableStyle ret {
    Style()
      .AlignItems(Align::Stretch)
      .AlignSelf(Align::FlexStart)
      .FlexDirection(FlexDirection::Column)
      .FlexGrow(0)
      // Hardcoded in the XAML source
      .Width(68),
  };
  return ret;
}

const auto& AppBarButtonGlyphStyle() {
  using StaticTheme::Generic::AppBarButtonContentHeight;
  static const ImmutableStyle ret {
    Style().Font(SystemFont::ResolveGlyphFont(AppBarButtonContentHeight))};
  return ret;
}

const auto& AppBarButtonLabelStyle() {
  using namespace StaticTheme::AppBarButton;
  static const ImmutableStyle ret {
    Style().Font(SystemFont::Resolve(12)).Margin(AppBarButtonTextLabelMargin)};
  return ret;
}

const auto& AppBarButtonBorderStyle() {
  using namespace PseudoClasses;
  using namespace StaticTheme::Common;
  using namespace StaticTheme::AppBarButton;

  static const ImmutableStyle ret {
    Style()
      .BackgroundColor(AppBarButtonBackground)
      .BorderColor(AppBarButtonBorderBrush)
      .Color(AppBarButtonForeground)
      .BorderRadius(ControlCornerRadius)
      .FlexDirection(FlexDirection::Column)
      // TODO: compact margin, overflow margin
      .Margin(AppBarButtonInnerBorderMargin)
      .And(
        Hover,
        Style()
          .BackgroundColor(AppBarButtonBackgroundPointerOver)
          .BorderColor(AppBarButtonBorderBrushPointerOver)
          .Color(AppBarButtonForegroundPointerOver))
      .And(
        Active,
        Style()
          .BackgroundColor(AppBarButtonBackgroundPressed)
          .BorderColor(AppBarButtonBorderBrushPressed)
          .Color(AppBarButtonForegroundPressed))
      .And(
        Disabled,
        Style()
          .BackgroundColor(AppBarButtonBackgroundDisabled)
          .BorderColor(AppBarButtonBorderBrushDisabled)
          .Color(AppBarButtonForegroundDisabled)),
  };
  return ret;
}

const auto& AppBarButtonContentContainerStyle() {
  using namespace StaticTheme::AppBarButton;
  static const ImmutableStyle ret {
    Style()
      .AlignItems(Align::Center)
      .FlexDirection(FlexDirection::Column)
      .FlexGrow(0)
      // In the XAML, the content is not a child of the border - it's a z-order
      // overlap. We don't currently support that, so we have to use a child,
      // and undo the spacing.
      .Margin(-AppBarButtonInnerBorderMargin)
      .Descendants(
        AppBarButtonGlyphStyleClass,
        Style()
          .Margin(AppBarButtonContentViewboxMargin)
          // XAML uses a grid where the behavior of margins between rows is
          // different to the behavior of margins between flexbox items
          .MarginBottom(0)
          // The XAML uses fixed-width + offsets to center; we just center it
          // instead.
          .MarginLeft(0))
      .And(
        AppBarButtonLabelRightStyleClass,
        Style()
          .FlexDirection(FlexDirection::Row)
          .Descendants(
            AppBarButtonLabelStyleClass,
            Style().Margin(AppBarButtonTextLabelOnRightMargin)))
      .And(
        AppBarButtonLabelCollapsedStyleClass, Style().Display(Display::None))};
  return ret;
}

}// namespace

AppBarButton::AppBarButton(Window* const window)
  : Button(window, AppBarButtonStyleClass, AppBarButtonStyle()) {
  this->SetStructuralChildren({
    mBorderContainer = new Widget(
      window, AppBarButtonBorderContainerStyleClass, AppBarButtonBorderStyle()),
  });
  mBorderContainer->SetStructuralChildren({
    mContentContainer = new Widget(
      window,
      AppBarButtonContentContainerStyleClass,
      AppBarButtonContentContainerStyle()),
  });
  mContentContainer->SetStructuralChildren({
    mGlyph
    = new Label(window, AppBarButtonGlyphStyleClass, AppBarButtonGlyphStyle()),
    mLabel
    = new Label(window, AppBarButtonLabelStyleClass, AppBarButtonLabelStyle()),
    // Not yet supported
    // mChevron = new Label(window, ...),
  });
}

AppBarButton::~AppBarButton() = default;

void AppBarButton::SetLabel(const std::string_view label) {
  mLabel->SetText(label);
}

void AppBarButton::SetGlyph(const std::string_view glyph) {
  mGlyph->SetText(glyph);
}

void AppBarButton::SetIsCompact(const bool value) {
  if (mIsCompact == value) {
    return;
  }
  mIsCompact = value;
  this->ToggleStyleClass(AppBarButtonCompactStyleClass, mIsCompact);
}

void AppBarButton::SetLabelPosition(const LabelPosition value) {
  if (mLabelPosition == value) {
    return;
  }

  mContentContainer->ToggleStyleClass(
    AppBarButtonLabelRightStyleClass, value == LabelPosition::Right);
  mContentContainer->ToggleStyleClass(
    AppBarButtonLabelCollapsedStyleClass, value == LabelPosition::Collapsed);
  mLabelPosition = value;
}

void AppBarButton::SetIsOutline(const bool value) {
  if (mIsOutline == value) {
    return;
  }
  mIsOutline = value;
  this->ToggleStyleClass(AppBarButtonOutlineStyleClass, mIsOutline);
}

}// namespace FredEmmott::GUI::Widgets