// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "CheckBox.hpp"

#include <FredEmmott/GUI/StaticTheme/CheckBox.hpp>
#include <FredEmmott/GUI/Widgets/Label.hpp>

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass CheckBoxStyleClass {"CheckBox"};
constexpr LiteralStyleClass CheckGlyphStyleClass {"CheckBox/CheckGlyph"};
constexpr LiteralStyleClass FosterParentStyleClass {"CheckBox/FosterParent"};

auto MakeCheckBoxStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  const Style DefaultCheckBoxStyle
    = Style()
        .BackgroundColor(CheckBoxBackgroundUnchecked)
        .BorderColor(CheckBoxBorderBrushUnchecked)
        .BorderRadius(ControlCornerRadius)
        .BorderWidth(CheckBoxBorderThickness)
        .Color(CheckBoxForegroundUnchecked)
        .Font(WidgetFont::ControlContent)
        .Height(CheckBoxHeight)
        .MarginLeft(-CheckBoxPaddingLeft)
        .MinWidth(CheckBoxMinWidth)
        .PaddingBottom(CheckBoxPaddingBottom)
        .PaddingLeft(CheckBoxPaddingLeft)
        .PaddingRight(CheckBoxPaddingRight)
        .PaddingTop(CheckBoxPaddingTop)
        .OutlineLeftOffset(7)
        .OutlineTopOffset(3)
        .OutlineRightOffset(7)
        .OutlineBottomOffset(3);

  const auto& UncheckedNormal = DefaultCheckBoxStyle;
  const Style UncheckedPointerOver
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedPointerOver)
        .BorderColor(CheckBoxBorderBrushUncheckedPointerOver)
        .Color(CheckBoxForegroundUncheckedPointerOver);
  const Style UncheckedPressed
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedPressed)
        .BorderColor(CheckBoxBorderBrushUncheckedPressed)
        .Color(CheckBoxForegroundUncheckedPressed);
  const Style UncheckedDisabled
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedDisabled)
        .BorderColor(CheckBoxBorderBrushUncheckedDisabled)
        .Color(CheckBoxForegroundUncheckedDisabled);
  const Style UncheckedStyle = UncheckedNormal
    + Style()
        .And(Hover, UncheckedPointerOver)
        .And(Active, UncheckedPressed)
        .And(Disabled, UncheckedDisabled);
  const Style CheckedNormal = DefaultCheckBoxStyle
    + Style()
        .BackgroundColor(CheckBoxBackgroundChecked)
        .BorderColor(CheckBoxBorderBrushChecked)
        .Color(CheckBoxForegroundChecked);
  const Style CheckedPointerOver
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedPointerOver)
        .BorderColor(CheckBoxBorderBrushCheckedPointerOver)
        .Color(CheckBoxForegroundCheckedPointerOver);
  const Style CheckPressed
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedPressed)
        .BorderColor(CheckBoxBorderBrushCheckedPressed)
        .Color(CheckBoxForegroundCheckedPressed);
  const Style CheckedDisabled
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedDisabled)
        .BorderColor(CheckBoxBorderBrushCheckedDisabled)
        .Color(CheckBoxForegroundCheckedDisabled);
  const Style CheckedStyle = CheckedNormal
    + Style()
        .And(Hover, CheckedPointerOver)
        .And(Active, CheckPressed)
        .And(Disabled, CheckedDisabled);

  return Style().And(Checked, CheckedStyle).And(!Checked, UncheckedStyle);
}

auto& CheckBoxStyles() {
  static const ImmutableStyle ret {MakeCheckBoxStyles()};
  return ret;
}

auto& FosterParentStyles() {
  using namespace StaticTheme::CheckBox;
  static const ImmutableStyle ret {
    Style()
      .PaddingBottom(CheckBoxPaddingBottom + 6)
      .PaddingLeft(CheckBoxPaddingLeft)
      .PaddingRight(CheckBoxPaddingRight)
      .PaddingTop(CheckBoxPaddingTop),
  };
  return ret;
}

auto& GlyphStyles() {
  using namespace PseudoClasses;

  static const ImmutableStyle ret {
    Style().TranslateX(4).TranslateY(-2).And(
      !Checked, Style().Display(YGDisplayNone)),
  };
  return ret;
}

auto MakeGlyphBackgroundStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  const auto DefaultGlyphStyles
    = Style()
        .BorderRadius(ControlCornerRadius)
        .BorderWidth(CheckBoxBorderThickness)
        .Font(
          SystemFont::ResolveGlyphFont(SystemFont::BodyStrong)
            .WithSize(CheckBoxGlyphSize),
          !important)
        .Height(CheckBoxSize)
        .Width(CheckBoxSize);
  const Style& UncheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUnchecked)
        .BorderColor(CheckBoxCheckBackgroundStrokeUnchecked)
        .Color(CheckBoxCheckGlyphForegroundUnchecked);
  const Style UncheckedPointerOverGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedPointerOver)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedPointerOver)
        .Color(CheckBoxCheckGlyphForegroundUncheckedPointerOver);
  const Style UncheckedPointerPressedGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedPressed)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedPressed)
        .Color(CheckBoxCheckGlyphForegroundUncheckedPressed);
  const Style UncheckedDisabledGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedDisabled)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedDisabled)
        .Color(CheckBoxCheckGlyphForegroundUncheckedDisabled);
  const Style UncheckedGlyphStyles = UncheckedNormalGlyphStyles
    + Style()
        .And(Hover, UncheckedPointerOverGlyphStyles)
        .And(Active, UncheckedPointerPressedGlyphStyles)
        .And(Disabled, UncheckedDisabledGlyphStyles);
  const Style& CheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillChecked)
        .BorderColor(CheckBoxCheckBackgroundStrokeChecked)
        .Color(CheckBoxCheckGlyphForegroundChecked);
  const Style CheckedPointerOverGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedPointerOver)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedPointerOver)
        .Color(CheckBoxCheckGlyphForegroundCheckedPointerOver);
  const Style CheckedPointerPressedGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedPressed)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedPressed)
        .Color(CheckBoxCheckGlyphForegroundCheckedPressed);
  const Style CheckedDisabledGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedDisabled)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedDisabled)
        .Color(CheckBoxCheckGlyphForegroundCheckedDisabled);
  const Style CheckedGlyphStyles = CheckedNormalGlyphStyles
    + Style()
        .And(Hover, CheckedPointerOverGlyphStyles)
        .And(Active, CheckedPointerPressedGlyphStyles)
        .And(Disabled, CheckedDisabledGlyphStyles);

  return Style()
    .And(Checked, CheckedGlyphStyles)
    .And(!Checked, UncheckedGlyphStyles);
}

auto& GlyphBackgroundStyles() {
  static const ImmutableStyle ret {MakeGlyphBackgroundStyles()};
  return ret;
}

}// namespace

CheckBox::CheckBox(std::size_t id)
  : Widget(
      id,
      LiteralStyleClass {"CheckBox"},
      CheckBoxStyles(),
      {*CheckBoxStyleClass}) {
  this->SetDirectChildren({
    (new Widget(
       0,
       LiteralStyleClass {"CheckBox/GlyphContainer"},
       GlyphBackgroundStyles()))
      ->SetChildren({
        (new Label(0, GlyphStyles(), {*CheckGlyphStyleClass}))
          ->SetText(StaticTheme::CheckBox::CheckBoxCheckedGlyph),
      }),
    mFosterParent = new Widget(1, FosterParentStyleClass, FosterParentStyles()),
  });
}

void CheckBox::Toggle() {
  SetIsChecked(!IsChecked());
  mChanged = true;
}

Widget::EventHandlerResult CheckBox::OnClick(const MouseEvent&) {
  this->Toggle();
  return EventHandlerResult::StopPropagation;
}

Widget* CheckBox::GetFosterParent() const noexcept {
  return mFosterParent;
}

Widget::ComputedStyleFlags CheckBox::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
