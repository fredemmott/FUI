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

Button::Button(Window* const window)
  : Button(
      window,
      ButtonStyleClass,
      ButtonStyles(),
      {PseudoClasses::ExplicitMouseButtonSink}) {}

Button::Button(
  Window* const window,
  const StyleClass primaryStyleClass,
  const ImmutableStyle& style,
  const StyleClasses& classes)
  : Widget(
      window,
      primaryStyleClass,
      style,
      classes + PseudoClasses::ExplicitMouseButtonSink),
    IInvocable(this) {}

Button::~Button() = default;

ImmutableStyle Button::MakeImmutableStyle(const Style& mixin) {
  return ImmutableStyle {
    StaticTheme::Button::DefaultButtonStyle()
      + Style()
          .AlignSelf(Align::FlexStart)
          .Descendants({}, Style().PointerEvents(PointerEvents::None))
      + mixin,
  };
}

void Button::Invoke() {
  this->MarkActivated();
}

Widget::ComputedStyleFlags Button::OnComputedStyleChange(
  const Style& style,
  const StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
