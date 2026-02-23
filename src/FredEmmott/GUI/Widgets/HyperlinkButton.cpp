// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "HyperlinkButton.hpp"

#include <FredEmmott/GUI/StaticTheme/HyperlinkButton.hpp>

namespace FredEmmott::GUI::Widgets {
HyperlinkButton::HyperlinkButton(std::size_t id, const StyleClasses& classes)
  : Widget(
      id,
      LiteralStyleClass {"HyperlinkButton"},
      StaticTheme::HyperlinkButton::DefaultHyperlinkButtonStyle(),
      classes) {}

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
