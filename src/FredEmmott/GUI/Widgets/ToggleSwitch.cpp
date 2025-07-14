// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitch.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>
#include <FredEmmott/GUI/StaticTheme/ToggleSwitch.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "ToggleSwitchKnob.hpp"
#include "WidgetList.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

namespace {
const auto ToggleSwitchStyleClass = StyleClass::Make("ToggleSwitch");
const auto ToggleSwitchContentStyleClass
  = StyleClass::Make("ToggleSwitchContent");
}// namespace

using namespace StaticTheme;
using namespace widget_detail;

ToggleSwitch::ToggleSwitch(std::size_t id)
  : Widget(id, {ToggleSwitchStyleClass}) {
  this->ChangeDirectChildren([this] {
    mKnob.reset(new ToggleSwitchKnob({}));
    mFosterParent.reset(new Widget({}, {ToggleSwitchContentStyleClass}));
  });
  mFosterParent->ReplaceExplicitStyles(Style().Display(YGDisplayContents));
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

Style ToggleSwitch::GetBuiltInStyles() const {
  using namespace StaticTheme::ToggleSwitch;
  using namespace PseudoClasses;
  static const Style ret
    = Style()
        .AlignSelf(YGAlignFlexStart)
        .Color(ToggleSwitchContentForeground)
        .FlexDirection(YGFlexDirectionRow)
        .MarginRight(ToggleSwitchPostContentMargin)
        .And(Disabled, Style().Color(ToggleSwitchContentForegroundDisabled));
  return {ret};
}
Widget::ComputedStyleFlags ToggleSwitch::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  using enum ComputedStyleFlags;
  return Widget::OnComputedStyleChange(style, state) | InheritableActiveState
    | InheritableHoverState;
}

Widget::EventHandlerResult ToggleSwitch::OnClick(const MouseEvent&) {
  this->SetIsOn(!this->IsOn());
  // This is used to detect user-triggered changes, not any change
  mChanged.Set();
  return EventHandlerResult::StopPropagation;
}

WidgetList ToggleSwitch::GetDirectChildren() const noexcept {
  return {mKnob.get(), mFosterParent.get()};
}

}// namespace FredEmmott::GUI::Widgets
