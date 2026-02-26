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
constexpr LiteralStyleClass ToggleSwitchStyleClass("ToggleSwitch");
constexpr LiteralStyleClass ToggleSwitchContentStyleClass(
  "ToggleSwitchContent");

auto& ToggleSwitchStyle() {
  using namespace StaticTheme::ToggleSwitch;
  using namespace PseudoClasses;
  static const ImmutableStyle ret {
    Style()
      .AlignSelf(YGAlignFlexStart)
      .Color(ToggleSwitchContentForeground)
      .FlexDirection(YGFlexDirectionRow)
      .MarginRight(ToggleSwitchPostContentMargin)
      .OutlineLeftOffset(7)
      .OutlineTopOffset(3)
      .OutlineRightOffset(7)
      .OutlineBottomOffset(3)
      .And(Disabled, Style().Color(ToggleSwitchContentForegroundDisabled)),
  };
  return ret;
}

auto& FosterParentStyle() {
  static const ImmutableStyle ret {
    Style().Display(YGDisplayContents),
  };
  return ret;
}
}// namespace

using namespace StaticTheme;
using namespace widget_detail;

ToggleSwitch::ToggleSwitch(const id_type id)
  : Widget(id, ToggleSwitchStyleClass, ToggleSwitchStyle()) {
  this->SetDirectChildren({
    new ToggleSwitchKnob({}),
    mFosterParent
    = new Widget({}, ToggleSwitchContentStyleClass, FosterParentStyle()),
  });
}

bool ToggleSwitch::IsOn() const noexcept {
  return this->IsChecked();
}

void ToggleSwitch::SetIsOn(const bool value) noexcept {
  if (value == IsOn()) {
    return;
  }
  this->SetIsChecked(value);
}

Widget::ComputedStyleFlags ToggleSwitch::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  using enum ComputedStyleFlags;
  return Widget::OnComputedStyleChange(style, state) | InheritableActiveState
    | InheritableHoverState;
}

void ToggleSwitch::Toggle() {
  this->SetIsOn(!this->IsOn());
  // This is used to detect user-triggered changes, not any change
  mWasChanged = true;
}

Widget::EventHandlerResult ToggleSwitch::OnClick(const MouseEvent&) {
  this->Toggle();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
