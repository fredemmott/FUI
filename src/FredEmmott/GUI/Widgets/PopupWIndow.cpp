// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PopupWindow.hpp"

#include "WidgetList.hpp"

namespace FredEmmott::GUI::Widgets {

PopupWindow::PopupWindow(std::size_t id) : Widget(id) {
}

WidgetList PopupWindow::GetDirectChildren() const noexcept {
  return WidgetList::MakeEmpty();
}
WidgetStyles PopupWindow::GetDefaultStyles() const {
  return {{.mDisplay = YGDisplayNone}};
}

}// namespace FredEmmott::GUI::Widgets