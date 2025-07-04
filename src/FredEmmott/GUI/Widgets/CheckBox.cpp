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
  mCheckGlyph->SetAdditionalBuiltInStyles({
    .mTranslateX = 4,
    .mTranslateY = -2,
  });
  mCheckGlyphBackground->SetChildren({mCheckGlyph});

  this->UpdateCheckGlyphStyles();
}

void CheckBox::UpdateCheckGlyphStyles() {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;
  static const Style DefaultGlyphStyles {
    .mBorderRadius = ControlCornerRadius,
    .mBorderWidth = CheckBoxBorderThickness,
    .mFont = {
      SystemFont::ResolveGlyphFont(SystemFont::BodyStrong).WithSize(CheckBoxGlyphSize),
      !important,
    },
    .mHeight = CheckBoxSize,
    .mWidth = CheckBoxSize,
  };
  static const Style& UncheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style {
      .mBackgroundColor = CheckBoxCheckBackgroundFillUnchecked,
      .mBorderColor = CheckBoxCheckBackgroundStrokeUnchecked,
      .mColor = CheckBoxCheckGlyphForegroundUnchecked,
    };
  static const Style UncheckedPointerOverGlyphStyles {
    .mBackgroundColor = CheckBoxCheckBackgroundFillUncheckedPointerOver,
    .mBorderColor = CheckBoxCheckBackgroundStrokeUncheckedPointerOver,
    .mColor = CheckBoxCheckGlyphForegroundUncheckedPointerOver,
  };
  static const Style UncheckedPointerPressedGlyphStyles {
    .mBackgroundColor = CheckBoxCheckBackgroundFillUncheckedPressed,
    .mBorderColor = CheckBoxCheckBackgroundStrokeUncheckedPressed,
    .mColor = CheckBoxCheckGlyphForegroundUncheckedPressed,
  };
  static const Style UncheckedDisabledGlyphStyles {
    .mBackgroundColor = CheckBoxCheckBackgroundFillUncheckedDisabled,
    .mBorderColor = CheckBoxCheckBackgroundStrokeUncheckedDisabled,
    .mColor = CheckBoxCheckGlyphForegroundUncheckedDisabled,
  };
  static const Style UncheckedGlyphStyles = UncheckedNormalGlyphStyles + Style{
    .mAnd = {
      {Hover, UncheckedPointerOverGlyphStyles},
      {Active, UncheckedPointerPressedGlyphStyles},
      {Disabled, UncheckedDisabledGlyphStyles},
    },
  };
  static const Style& CheckedNormalGlyphStyles = DefaultGlyphStyles
    + Style {
      .mBackgroundColor = CheckBoxCheckBackgroundFillChecked,
      .mBorderColor = CheckBoxCheckBackgroundStrokeChecked,
      .mColor = CheckBoxCheckGlyphForegroundChecked,
    };
  static const Style CheckedPointerOverGlyphStyles {
    .mBackgroundColor = CheckBoxCheckBackgroundFillCheckedPointerOver,
    .mBorderColor = CheckBoxCheckBackgroundStrokeCheckedPointerOver,
    .mColor = CheckBoxCheckGlyphForegroundCheckedPointerOver,
  };
  static const Style CheckedPointerPressedGlyphStyles {
    .mBackgroundColor = CheckBoxCheckBackgroundFillCheckedPressed,
    .mBorderColor = CheckBoxCheckBackgroundStrokeCheckedPressed,
    .mColor = CheckBoxCheckGlyphForegroundCheckedPressed,
  };
  static const Style CheckedDisabledGlyphStyles {
    .mBackgroundColor = CheckBoxCheckBackgroundFillCheckedDisabled,
    .mBorderColor = CheckBoxCheckBackgroundStrokeCheckedDisabled,
    .mColor = CheckBoxCheckGlyphForegroundCheckedDisabled,
  };
  static const Style CheckedGlyphStyles = CheckedNormalGlyphStyles + Style{
    .mAnd = {
      {Hover, CheckedPointerOverGlyphStyles},
      {Active, CheckedPointerPressedGlyphStyles},
      {Disabled, CheckedDisabledGlyphStyles},
    },
  };

  switch (mState) {
    case State::Checked:
      mCheckGlyphBackground->AddExplicitStyles(CheckedGlyphStyles);
      // For now, just using the 'fallback' icon; it would be better to
      // port AnimatedAcceptVisualSource
      mCheckGlyph->SetText(CheckBoxCheckedGlyph);
      break;
    case State::Unchecked:
      mCheckGlyphBackground->AddExplicitStyles(UncheckedGlyphStyles);
      mCheckGlyph->SetText({});
      break;
  }
}

bool CheckBox::IsChecked() const noexcept {
  return mState == State::Checked;
}

void CheckBox::SetIsChecked(const bool checked) noexcept {
  if (checked == IsChecked()) {
    return;
  }
  mState = checked ? State::Checked : State::Unchecked;
  this->UpdateCheckGlyphStyles();
}

Style CheckBox::GetBuiltInStyles() const {
  using namespace StaticTheme::Common;
  using namespace StaticTheme::CheckBox;
  using namespace PseudoClasses;

  static const Style DefaultCheckBoxStyle {
    .mBackgroundColor = CheckBoxBackgroundUnchecked,
    .mBorderColor = CheckBoxBorderBrushUnchecked,
    .mBorderRadius = ControlCornerRadius,
    .mBorderWidth = CheckBoxBorderThickness,
    .mColor = CheckBoxForegroundUnchecked,
    .mFlexGrow = 0,
    .mFont = {WidgetFont::ControlContent},
    .mHeight = CheckBoxHeight,
    .mMinWidth = CheckBoxMinWidth,
    .mPaddingBottom = CheckBoxPaddingBottom,
    .mPaddingLeft = CheckBoxPaddingLeft,
    .mPaddingRight = CheckBoxPaddingRight,
    .mPaddingTop = CheckBoxPaddingTop,
  };

  static const auto& UncheckedNormal = DefaultCheckBoxStyle;
  static const Style UncheckedPointerOver {
    .mBackgroundColor = CheckBoxBackgroundUncheckedPointerOver,
    .mBorderColor = CheckBoxBorderBrushUncheckedPointerOver,
    .mColor = CheckBoxForegroundUncheckedPointerOver,
  };
  static const Style UncheckedPressed {
    .mBackgroundColor = CheckBoxBackgroundUncheckedPressed,
    .mBorderColor = CheckBoxBorderBrushUncheckedPressed,
    .mColor = CheckBoxForegroundUncheckedPressed,
  };
  static const Style UncheckedDisabled {
    .mBackgroundColor = CheckBoxBackgroundUncheckedDisabled,
    .mBorderColor = CheckBoxBorderBrushUncheckedDisabled,
    .mColor = CheckBoxForegroundUncheckedDisabled,
  };
  static const Style UncheckedStyle = UncheckedNormal + Style {
    .mAnd = {
      {Hover, UncheckedPointerOver},
      {Active, UncheckedPressed},
      {Disabled, UncheckedDisabled},
    },
  };
  static const Style CheckedNormal = DefaultCheckBoxStyle
    + Style {
      .mBackgroundColor = CheckBoxBackgroundChecked,
      .mBorderColor = CheckBoxBorderBrushChecked,
      .mColor = CheckBoxForegroundChecked,
    };
  static const Style CheckedPointerOver {
    .mBackgroundColor = CheckBoxBackgroundCheckedPointerOver,
    .mBorderColor = CheckBoxBorderBrushCheckedPointerOver,
    .mColor = CheckBoxForegroundCheckedPointerOver,
  };
  static const Style CheckPressed {
    .mBackgroundColor = CheckBoxBackgroundCheckedPressed,
    .mBorderColor = CheckBoxBorderBrushCheckedPressed,
    .mColor = CheckBoxForegroundCheckedPressed,
  };
  static const Style CheckedDisabled {
    .mBackgroundColor = CheckBoxBackgroundCheckedDisabled,
    .mBorderColor = CheckBoxBorderBrushCheckedDisabled,
    .mColor = CheckBoxForegroundCheckedDisabled,
  };
  static const Style CheckedStyle = CheckedNormal + Style {
    .mAnd = {
      {Hover, CheckedPointerOver},
      {Active, CheckPressed},
      {Disabled, CheckedDisabled},
    },
  };

  return IsChecked() ? CheckedStyle : UncheckedStyle;
}

Widget::EventHandlerResult CheckBox::OnClick(const MouseEvent&) {
  SetIsChecked(!IsChecked());
  mChanged.Set();
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

}// namespace FredEmmott::GUI::Widgets
