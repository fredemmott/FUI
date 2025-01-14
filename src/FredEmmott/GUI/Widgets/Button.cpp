// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

namespace {
const auto ButtonStyleClass = Style::Class::Make("Button");
}

Button::Button(std::size_t id) : Widget(id, {ButtonStyleClass}) {}

WidgetStyles Button::GetBuiltInStyles() const {
  using namespace StaticTheme::Button;

  static const WidgetStyles ret {
    .mBase = {
      .mAlignSelf = YGAlignFlexStart,
      .mBackgroundColor = ButtonBackground,
      .mBorderColor = ButtonBorderBrush,
      .mBorderRadius = ControlCornerRadius,
      .mBorderWidth = ButtonBorderThemeThickness,
      .mColor = ButtonForeground,
      .mFont = WidgetFont::ControlContent,
      .mPaddingBottom = ButtonPaddingBottom,
      .mPaddingLeft = ButtonPaddingLeft,
      .mPaddingRight = ButtonPaddingRight,
      .mPaddingTop = ButtonPaddingTop,
    },
    .mDisabled = {
      .mBackgroundColor = ButtonBackgroundDisabled,
      .mBorderColor = ButtonBorderBrushDisabled,
      .mColor = ButtonForegroundDisabled,
    },
    .mHover = {
      .mBackgroundColor = ButtonBackgroundPointerOver,
      .mBorderColor = ButtonBorderBrushPointerOver,
      .mColor = ButtonForegroundPointerOver,
    },
    .mActive = {
      .mBackgroundColor = ButtonBackgroundPressed,
      .mBorderColor = ButtonBorderBrushPressed,
      .mColor = ButtonForegroundPressed,
    },
  };
  return ret;
}

Widget::EventHandlerResult Button::OnClick(MouseEvent* e) {
  mClicked.Set();
  return EventHandlerResult::StopPropagation;
}

Widget::ComputedStyleFlags Button::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  return ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
