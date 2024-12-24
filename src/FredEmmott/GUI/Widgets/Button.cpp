// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

Button::Button(std::size_t id) : Widget(id) {
}

WidgetStyles Button::GetDefaultStyles() const {
  constexpr auto VerticalPadding = Spacing;
  constexpr auto HorizontalPadding = Spacing * 3;

  static const WidgetStyles ret {
    .mBase = {
      .mAlignSelf = YGAlignFlexStart,
      .mBackgroundColor = WidgetColor::ControlFillDefault,
      // TODO: should be 'ControlElevationBorderBrush' linear gradient
      .mBorderColor = WidgetColor::ControlStrokeDefault,
      .mBorderRadius = Spacing,
      .mBorderWidth = Spacing / 4,
      .mColor = WidgetColor::TextFillPrimary,
      .mFont = WidgetFont::ControlContent,
      .mPaddingBottom = VerticalPadding,
      .mPaddingLeft = HorizontalPadding,
      .mPaddingRight = HorizontalPadding,
      .mPaddingTop = VerticalPadding,
    },
    .mHover = {
      .mBackgroundColor = WidgetColor::ControlFillSecondary,
    },
    .mActive = {
      .mBackgroundColor = WidgetColor::ControlFillTertiary,
      .mBorderColor = WidgetColor::ControlStrokeDefault,
      .mColor = WidgetColor::TextFillSecondary,
    },
  };
  return ret;
}
Widget::EventHandlerResult Button::OnClick(MouseEvent* e) {
  mClicked.Set();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
