// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "HyperlinkButton.hpp"

#include <FredEmmott/GUI/StaticTheme/HyperlinkButton.hpp>

namespace FredEmmott::GUI::Widgets {
HyperlinkButton::HyperlinkButton(
  Window* const window,
  const StyleClasses& classes)
  : Widget(
      window,
      LiteralStyleClass {"HyperlinkButton"},
      StaticTheme::HyperlinkButton::DefaultHyperlinkButtonStyle(),
      classes),
    IInvocable(this) {}

HyperlinkButton::~HyperlinkButton() = default;

void HyperlinkButton::Invoke() {
  mWasActivated = true;
}

Widget::EventHandlerResult HyperlinkButton::OnClick(const MouseEvent&) {
  this->Invoke();
  return EventHandlerResult::StopPropagation;
}

Widget::ComputedStyleFlags HyperlinkButton::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  return Widget::OnComputedStyleChange(style, state)
    | ComputedStyleFlags::InheritableActiveState
    | ComputedStyleFlags::InheritableHoverState;
}
}// namespace FredEmmott::GUI::Widgets
