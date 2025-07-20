// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "CheckBox.hpp"

#include <FredEmmott/GUI/StaticTheme/CheckBox.hpp>

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

CheckBox::CheckBox(std::size_t id) : Widget(id) {
  static const auto CheckGlyphClass = StyleClass::Make("CheckBox_CheckGlyph");
  static const auto FosterParentClass
    = StyleClass::Make("CheckBox_FosterParent");
  this->ChangeDirectChildren([this] {
    mCheckGlyphBackground = std::make_unique<Widget>(0);
    mFosterParent
      = std::make_unique<Widget>(1, StyleClasses {FosterParentClass});
  });

  mCheckGlyph = new Label(0, StyleClasses {CheckGlyphClass});
  mCheckGlyphBackground->SetChildren({mCheckGlyph});

  using namespace StaticTheme::CheckBox;
  mFosterParent->BuiltInStyles()
    = Style()
        .PaddingBottom(CheckBoxPaddingBottom + 6)
        .PaddingLeft(CheckBoxPaddingLeft)
        .PaddingRight(CheckBoxPaddingRight)
        .PaddingTop(CheckBoxPaddingTop);

  this->UpdateStyles();
}

void CheckBox::UpdateCheckGlyphStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  static const auto DefaultGlyphStyles
    = Style()
        .BorderRadius(ControlCornerRadius)
        .BorderWidth(CheckBoxBorderThickness)
        .Font(
          SystemFont::ResolveGlyphFont(SystemFont::BodyStrong)
            .WithSize(CheckBoxGlyphSize),
          !important)
        .Height(CheckBoxSize)
        .Width(CheckBoxSize);
  static const Style& UncheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUnchecked)
        .BorderColor(CheckBoxCheckBackgroundStrokeUnchecked)
        .Color(CheckBoxCheckGlyphForegroundUnchecked);
  static const Style UncheckedPointerOverGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedPointerOver)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedPointerOver)
        .Color(CheckBoxCheckGlyphForegroundUncheckedPointerOver);
  static const Style UncheckedPointerPressedGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedPressed)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedPressed)
        .Color(CheckBoxCheckGlyphForegroundUncheckedPressed);
  static const Style UncheckedDisabledGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillUncheckedDisabled)
        .BorderColor(CheckBoxCheckBackgroundStrokeUncheckedDisabled)
        .Color(CheckBoxCheckGlyphForegroundUncheckedDisabled);
  static const Style UncheckedGlyphStyles = UncheckedNormalGlyphStyles
    + Style()
        .And(Hover, UncheckedPointerOverGlyphStyles)
        .And(Active, UncheckedPointerPressedGlyphStyles)
        .And(Disabled, UncheckedDisabledGlyphStyles);
  static const Style& CheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillChecked)
        .BorderColor(CheckBoxCheckBackgroundStrokeChecked)
        .Color(CheckBoxCheckGlyphForegroundChecked);
  static const Style CheckedPointerOverGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedPointerOver)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedPointerOver)
        .Color(CheckBoxCheckGlyphForegroundCheckedPointerOver);
  static const Style CheckedPointerPressedGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedPressed)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedPressed)
        .Color(CheckBoxCheckGlyphForegroundCheckedPressed);
  static const Style CheckedDisabledGlyphStyles
    = Style()
        .BackgroundColor(CheckBoxCheckBackgroundFillCheckedDisabled)
        .BorderColor(CheckBoxCheckBackgroundStrokeCheckedDisabled)
        .Color(CheckBoxCheckGlyphForegroundCheckedDisabled);
  static const Style CheckedGlyphStyles = CheckedNormalGlyphStyles
    + Style()
        .And(Hover, CheckedPointerOverGlyphStyles)
        .And(Active, CheckedPointerPressedGlyphStyles)
        .And(Disabled, CheckedDisabledGlyphStyles);
  mCheckGlyph->SetText(CheckBoxCheckedGlyph);

  mCheckGlyphBackground->BuiltInStyles()
    = Style()
        .And(Checked, CheckedGlyphStyles)
        .And(!Checked, UncheckedGlyphStyles);
  mCheckGlyph->BuiltInStyles() = Style().TranslateX(4).TranslateY(-2).And(
    !Checked, Style().Display(YGDisplayNone));
}

void CheckBox::UpdateStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  static const Style DefaultCheckBoxStyle
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
        .PaddingTop(CheckBoxPaddingTop);

  static const auto& UncheckedNormal = DefaultCheckBoxStyle;
  static const Style UncheckedPointerOver
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedPointerOver)
        .BorderColor(CheckBoxBorderBrushUncheckedPointerOver)
        .Color(CheckBoxForegroundUncheckedPointerOver);
  static const Style UncheckedPressed
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedPressed)
        .BorderColor(CheckBoxBorderBrushUncheckedPressed)
        .Color(CheckBoxForegroundUncheckedPressed);
  static const Style UncheckedDisabled
    = Style()
        .BackgroundColor(CheckBoxBackgroundUncheckedDisabled)
        .BorderColor(CheckBoxBorderBrushUncheckedDisabled)
        .Color(CheckBoxForegroundUncheckedDisabled);
  static const Style UncheckedStyle = UncheckedNormal
    + Style()
        .And(Hover, UncheckedPointerOver)
        .And(Active, UncheckedPressed)
        .And(Disabled, UncheckedDisabled);
  static const Style CheckedNormal = DefaultCheckBoxStyle
    + Style()
        .BackgroundColor(CheckBoxBackgroundChecked)
        .BorderColor(CheckBoxBorderBrushChecked)
        .Color(CheckBoxForegroundChecked);
  static const Style CheckedPointerOver
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedPointerOver)
        .BorderColor(CheckBoxBorderBrushCheckedPointerOver)
        .Color(CheckBoxForegroundCheckedPointerOver);
  static const Style CheckPressed
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedPressed)
        .BorderColor(CheckBoxBorderBrushCheckedPressed)
        .Color(CheckBoxForegroundCheckedPressed);
  static const Style CheckedDisabled
    = Style()
        .BackgroundColor(CheckBoxBackgroundCheckedDisabled)
        .BorderColor(CheckBoxBorderBrushCheckedDisabled)
        .Color(CheckBoxForegroundCheckedDisabled);
  static const Style CheckedStyle = CheckedNormal
    + Style()
        .And(Hover, CheckedPointerOver)
        .And(Active, CheckPressed)
        .And(Disabled, CheckedDisabled);

  BuiltInStyles()
    = Style().And(Checked, CheckedStyle).And(!Checked, UncheckedStyle);
  this->UpdateCheckGlyphStyles();
}

Widget::EventHandlerResult CheckBox::OnClick(const MouseEvent&) {
  SetIsChecked(!IsChecked());
  mChanged = true;
  return EventHandlerResult::StopPropagation;
}

Widget* CheckBox::GetFosterParent() const noexcept {
  return mFosterParent.get();
}

WidgetList CheckBox::GetDirectChildren() const noexcept {
  return {
    mCheckGlyphBackground.get(),
    mFosterParent.get(),
  };
}
Widget::ComputedStyleFlags CheckBox::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
