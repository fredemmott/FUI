// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "Button.hpp"

#include <ranges>

#include "FredEmmott/utility/lazy_init.hpp"

using namespace FredEmmott::utility;

namespace FredEmmott::GUI::Widgets {

Button::Button(std::size_t id) : Widget(id) {
}

Widget* Button::GetChild() const noexcept {
  return mLabel.get();
}

void Button::SetChild(Widget* child) {
  if (child == mLabel.get()) {
    return;
  }

  const auto layout = this->GetLayoutNode();
  if (mLabel) {
    YGNodeRemoveChild(layout, mLabel->GetLayoutNode());
  }
  mLabel.reset(child);
  YGNodeInsertChild(this->GetLayoutNode(), child->GetLayoutNode(), 0);
}

std::span<Widget* const> Button::GetChildren() const noexcept {
  if (!mLabel) {
    return {};
  }

  mLabelRawPointer = mLabel.get();
  return {&mLabelRawPointer, 1};
}

WidgetStyles Button::GetDefaultStyles() const {
  constexpr auto VerticalPadding = Spacing;
  constexpr auto HorizontalPadding = Spacing * 3;

  static const WidgetStyles ret {
    .mDefault = {
      .mBackgroundColor = WidgetColor::ControlFillDefault,
      .mBorderColor = WidgetColor::ControlElevationBorder,
      .mBorderRadius = Spacing,
      .mBorderWidth = Spacing / 4,
      .mPaddingBottom = VerticalPadding,
      .mPaddingLeft = HorizontalPadding,
      .mPaddingRight = HorizontalPadding,
      .mPaddingTop = VerticalPadding,
    },
    .mHover = {
      .mBackgroundColor = SK_ColorRED,
    },
  };
  return ret;
}

}// namespace FredEmmott::GUI::Widgets
