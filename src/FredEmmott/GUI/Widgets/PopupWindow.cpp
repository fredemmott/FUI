// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {

const auto& InvisibleStyle() {
  static const ImmutableStyle ret {
    Style().Display(Display::None),
  };
  return ret;
}

}// namespace

PopupWindow::PopupWindow(Window* const window)
  : Widget(
      window,
      LiteralStyleClass {"PopupWindow"},
      InvisibleStyle(),
      {PseudoClasses::LayoutOrphan}),
    mWindow(window->CreatePopup()) {}

PopupWindow::~PopupWindow() = default;

Widget::ComputedStyleFlags PopupWindow::OnComputedStyleChange(
  const Style& style,
  const StateFlags flags) {
  auto ret = Widget::OnComputedStyleChange(style, flags);
  if (mWindow->GetFrameRateRequirement().RequiresSmoothAnimation()) {
    ret |= ComputedStyleFlags::Animating;
  }
  return ret;
}
FrameRateRequirement PopupWindow::GetFrameRateRequirement() const noexcept {
  return Widget::GetFrameRateRequirement() + mWindow->GetFrameRateRequirement();
}

}// namespace FredEmmott::GUI::Widgets