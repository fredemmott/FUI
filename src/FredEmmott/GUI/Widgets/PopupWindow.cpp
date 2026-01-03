// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include "FredEmmott/GUI/detail/immediate_detail.hpp"
#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {

const auto& InvisibleStyle() {
  static const ImmutableStyle ret {
    Style().Display(YGDisplayNone),
  };
  return ret;
}

}// namespace

PopupWindow::PopupWindow(const std::size_t id)
  : Widget(id, InvisibleStyle(), {PseudoClasses::LayoutOrphan}),
    mWindow(Immediate::immediate_detail::tWindow->CreatePopup()) {}

Widget::ComputedStyleFlags PopupWindow::OnComputedStyleChange(
  const Style& style,
  StateFlags flags) {
  auto ret = Widget::OnComputedStyleChange(style, flags);
  if (mWindow->GetFrameRateRequirement().RequiresSmoothAnimation()) {
    ret |= ComputedStyleFlags::Animating;
  }
  return ret;
}

}// namespace FredEmmott::GUI::Widgets