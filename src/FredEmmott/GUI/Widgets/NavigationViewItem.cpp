// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationViewItem.hpp"

namespace FredEmmott::GUI::Widgets {

static constexpr LiteralStyleClass NavigationViewItemStyleClass {
  "NavigationView/Item"};

NavigationViewItem::NavigationViewItem(const id_type id)
  : Widget(id, NavigationViewItemStyleClass, {}) {}

NavigationViewItem::~NavigationViewItem() = default;

Widget::EventHandlerResult NavigationViewItem::OnClick(const MouseEvent&) {
  mWasActivated = true;
  this->SetIsChecked(true);
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
