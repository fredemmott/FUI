// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <FredEmmott/GUI/Widgets/ScrollView.hpp>
#include <FredEmmott/GUI/Widgets/WidgetList.hpp>

namespace FredEmmott::GUI::Widgets {

ScrollView::ScrollView(std::size_t id, const StyleClasses& classes)
  : Widget(id, classes) {}

ScrollView::~ScrollView() = default;

WidgetList ScrollView::GetDirectChildren() const noexcept {
  return Widget::GetDirectChildren();
}

}// namespace FredEmmott::GUI::Widgets