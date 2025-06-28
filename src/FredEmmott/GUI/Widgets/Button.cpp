// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

namespace {
const auto ButtonStyleClass = StyleClass::Make("Button");
}

Button::Button(std::size_t id) : Widget(id, {ButtonStyleClass}) {}

Style Button::GetBuiltInStyles() const {
  using namespace StaticTheme::Button;

  using namespace PseudoClasses;
  static const Style ret {
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
    .mAnd = {
      { Disabled, Style {
        .mBackgroundColor = ButtonBackgroundDisabled,
        .mBorderColor = ButtonBorderBrushDisabled,
        .mColor = ButtonForegroundDisabled,
      }},
      { Hover, Style {
      .mBackgroundColor = ButtonBackgroundPointerOver,
      .mBorderColor = ButtonBorderBrushPointerOver,
      .mColor = ButtonForegroundPointerOver,
      }},
      { Active, Style {
        .mBackgroundColor = ButtonBackgroundPressed,
        .mBorderColor = ButtonBorderBrushPressed,
        .mColor = ButtonForegroundPressed,
      }},
    },
  };
  return ret;
}

Widget::EventHandlerResult Button::OnClick(const MouseEvent&) {
  mClicked.Set();
  return EventHandlerResult::StopPropagation;
}
Widget::ComputedStyleFlags Button::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
