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
const auto ToggleSwitchStyleClass = Style::Class::Make("ToggleSwitch");
const auto ToggleSwitchContentStyleClass
  = Style::Class::Make("ToggleSwitchContent");
}// namespace

using namespace StaticTheme;
using namespace widget_detail;

ToggleSwitch::ToggleSwitch(std::size_t id)
  : Widget(id, {ToggleSwitchStyleClass}) {
  this->ChangeDirectChildren([this] {
    mKnob.reset(new ToggleSwitchKnob({}));
    mFosterParent.reset(new Widget({}, {ToggleSwitchContentStyleClass}));
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

WidgetStyles ToggleSwitch::GetBuiltInStyles() const {
  using namespace StaticTheme::ToggleSwitch;
  using enum Style::PseudoClass;
  static const Style ret {
    .mAlignSelf = YGAlignFlexStart,
    .mColor = ToggleSwitchContentForeground,
    .mFlexDirection = YGFlexDirectionRow,
    .mMarginRight = ToggleSwitchPostContentMargin,
    .mAnd = {
      { Disabled, Style {
        .mColor = ToggleSwitchContentForegroundDisabled,
      }},
    },
  };
  return {ret};
}
Widget::ComputedStyleFlags ToggleSwitch::OnComputedStyleChange(
  const Style& style,
  StateFlags state) {
  using enum ComputedStyleFlags;
  return Widget::OnComputedStyleChange(style, StateFlags::Animating)
    | InheritableActiveState | InheritableHoverState;
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
