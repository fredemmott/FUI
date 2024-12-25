// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "ToggleSwitchKnob.hpp"

#include <FredEmmott/GUI/StaticTheme.hpp>

#include "FredEmmott/GUI/detail/widget_detail.hpp"
#include "ToggleSwitchThumb.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

using namespace StaticTheme;
using namespace widget_detail;

ToggleSwitchKnob::ToggleSwitchKnob(std::size_t id) : Widget(id) {
  this->SetChildren({new ToggleSwitchThumb({})});
}

WidgetStyles ToggleSwitchKnob::GetDefaultStyles() const {
  using namespace StaticTheme;
  static const WidgetStyles baseStyles {
    .mBase = {
      .mBorderRadius = Spacing * 2.5f,
      .mBorderWidth = 1,
      .mFlexDirection = YGFlexDirectionColumn,
      .mHeight = Spacing * 5,
      .mWidth = Spacing * 10,
    },
  };
  static const WidgetStyles offStyles {
    .mBase = {
      .mBackgroundColor = ControlAltFillColorSecondaryBrush,
      .mBorderColor = ControlStrongStrokeColorDefaultBrush,
    },
    .mHover = {
      .mBackgroundColor = ControlAltFillColorTertiaryBrush,
    },
    .mActive = {
      .mBackgroundColor = ControlAltFillColorQuarternaryBrush,
    },
  };
  static const WidgetStyles onStyles {
    .mBase = {
      .mBackgroundColor = AccentFillColorDefaultBrush,
      .mBorderColor = AccentFillColorDefaultBrush,
    },
    .mHover = {
      .mBackgroundColor = AccentFillColorSecondaryBrush,
      .mBorderColor = AccentFillColorSecondaryBrush,
    },
    .mActive = {
      .mBackgroundColor = AccentFillColorTertiaryBrush,
      .mBorderColor = AccentFillColorTertiaryBrush,
    },
  };
  return baseStyles + (mIsOn ? onStyles : offStyles);
}

Widget::EventHandlerResult ToggleSwitchKnob::OnClick(MouseEvent* e) {
  mChanged.Set();
  mIsOn = !mIsOn;
  const auto children = this->GetChildren();
  if (children.size() > 0) {
    if (auto thumb = widget_cast<ToggleSwitchThumb>(children.front())) {
      thumb->mIsOn = mIsOn;
    }
  }
  return EventHandlerResult::StopPropagation;
}

Widget::ComputedStyleFlags ToggleSwitchKnob::OnComputedStyleChange(
  const Style&) {
  using enum ComputedStyleFlags;
  return InheritableActiveState | InheritableHoverState;
}

}// namespace FredEmmott::GUI::Widgets
