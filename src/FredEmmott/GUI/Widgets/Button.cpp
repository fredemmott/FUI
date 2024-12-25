// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

Button::Button(std::size_t id) : Widget(id) {
}

WidgetStyles Button::GetDefaultStyles() const {
  constexpr auto VerticalPadding = Spacing;
  constexpr auto HorizontalPadding = Spacing * 3;

  static const WidgetStyles ret {
    .mBase = {
      .mAlignSelf = YGAlignFlexStart,
      .mBackgroundColor = ControlFillColorDefaultBrush,
      .mBorderColor = Brush { ControlElevationBorderBrush },
      .mBorderRadius = Spacing,
      .mBorderWidth = Spacing / 4,
      .mColor = TextFillColorPrimaryBrush,
      .mFont = WidgetFont::ControlContent,
      .mPaddingBottom = VerticalPadding,
      .mPaddingLeft = HorizontalPadding,
      .mPaddingRight = HorizontalPadding,
      .mPaddingTop = VerticalPadding,
    },
    .mHover = {
      .mBackgroundColor = ControlFillColorSecondaryBrush,
      .mBorderColor = ControlElevationBorderBrush,
    },
    .mActive = {
      .mBackgroundColor = ControlFillColorTertiaryBrush,
      .mBorderColor = ControlStrokeColorDefaultBrush,
      .mColor = TextFillColorSecondaryBrush,
    },
  };
  return ret;
}

Widget::EventHandlerResult Button::OnClick(MouseEvent* e) {
  mClicked.Set();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
