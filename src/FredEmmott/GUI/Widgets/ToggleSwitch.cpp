// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitch.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "ToggleSwitchKnob.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme;
using namespace widget_detail;

ToggleSwitch::ToggleSwitch(std::size_t id) : Widget(id) {
  this->ChangeDirectChildren([this] {
    mKnob.reset(new ToggleSwitchKnob({}));
    mFosterParent.reset(new Widget({}));
  });
  mFosterParent->SetExplicitStyles({
    .mBase = {
      .mDisplay = YGDisplayContents,
    },
  });
}

bool ToggleSwitch::IsOn() const noexcept {
  return mKnob->IsOn();
}

void ToggleSwitch::SetIsOn(bool value) noexcept {
  if (value == IsOn()) {
    return;
  }
  mKnob->SetIsOn(value);
}

WidgetStyles ToggleSwitch::GetDefaultStyles() const {
  static const WidgetStyles ret {
    .mBase = {
      .mAlignSelf = YGAlignFlexStart,
      .mFlexDirection = YGFlexDirectionRow,
      .mGap = Spacing * 2,
    },
  };
  return ret;
}
Widget::ComputedStyleFlags ToggleSwitch::OnComputedStyleChange(
  const Style& style) {
  using enum ComputedStyleFlags;
  return Widget::OnComputedStyleChange(style) | InheritableActiveState
    | InheritableHoverState;
}

Widget::EventHandlerResult ToggleSwitch::OnClick(MouseEvent* event) {
  this->SetIsOn(!this->IsOn());
  // This is used to detect user-triggered changes, not any change
  mChanged.Set();
  return EventHandlerResult::StopPropagation;
}

WidgetList ToggleSwitch::GetDirectChildren() const noexcept {
  return {mKnob.get(), mFosterParent.get()};
}

}// namespace FredEmmott::GUI::Widgets
