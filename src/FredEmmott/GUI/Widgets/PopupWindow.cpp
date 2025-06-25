// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

PopupWindow::PopupWindow(std::size_t id)
  : Widget(id),
    mWindow(std::move(Immediate::immediate_detail::tWindow->CreatePopup())) {}

WidgetList PopupWindow::GetDirectChildren() const noexcept {
  return WidgetList::MakeEmpty();
}

Style PopupWindow::GetBuiltInStyles() const {
  return {.mDisplay = YGDisplayNone};
}

Widget::ComputedStyleFlags PopupWindow::OnComputedStyleChange(
  const Style& style,
  StateFlags flags) {
  auto ret = Widget::OnComputedStyleChange(style, flags);
  if (
    mWindow->GetFrameRateRequirement()
    == FrameRateRequirement::SmoothAnimation) {
    ret |= ComputedStyleFlags::Animating;
  }
  return ret;
}

}// namespace FredEmmott::GUI::Widgets