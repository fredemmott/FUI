// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/Button.hpp>

using namespace FredEmmott::utility;
using namespace FredEmmott::GUI::StaticTheme;

namespace FredEmmott::GUI::Widgets {

namespace {
constexpr LiteralStyleClass ButtonStyleClass("Button");
auto& ButtonStyles() {
  static const auto ret = Button::MakeImmutableStyle({});
  return ret;
}
}// namespace

Button::Button(const std::size_t id)
  : Button(id, ButtonStyles(), {*ButtonStyleClass}) {}

Button::Button(
  const std::size_t id,
  const ImmutableStyle& style,
  const StyleClasses& classes)
  : Widget(id, style, classes) {}

ImmutableStyle Button::MakeImmutableStyle(const Style& mixin) {
  return ImmutableStyle {
    StaticTheme::Button::DefaultButtonStyle()
      + Style()
          .AlignSelf(YGAlignFlexStart)
          .Descendants({}, Style().PointerEvents(PointerEvents::None))
      + mixin,
  };
}

Widget::EventHandlerResult Button::OnClick(const MouseEvent&) {
  mClicked = true;
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
