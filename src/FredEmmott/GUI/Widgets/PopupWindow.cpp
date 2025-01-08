// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

PopupWindow::PopupWindow(std::size_t id) : Widget(id) {}

WidgetList PopupWindow::GetDirectChildren() const noexcept {
  return WidgetList::MakeEmpty();
}
WidgetStyles PopupWindow::GetBuiltInStyles() const {
  return {{.mDisplay = YGDisplayNone}};
}

Widget::ComputedStyleFlags PopupWindow::OnComputedStyleChange(
  const Style& style) {
  auto flags = Widget::OnComputedStyleChange(style);
  if (
    mWindow.GetFrameRateRequirement()
    == FrameRateRequirement::SmoothAnimation) {
    flags |= ComputedStyleFlags::Animating;
  }
  return flags;
}

}// namespace FredEmmott::GUI::Widgets