// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationViewItem.hpp"

#include "FredEmmott/GUI/StaticTheme/detail/NavigationView.handwritten.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace StaticTheme::NavigationView;

constexpr LiteralStyleClass NavigationViewItemStyleClass {
  "NavigationView/Item"};
}// namespace

NavigationViewItem::NavigationViewItem(const id_type id)
  : Widget(id, NavigationViewItemStyleClass, NavigationViewItemStyle()),
    mIcon(new Label(0, NavigationViewItemIconStyle())),
    mText(new Label(
      0,
      NavigationViewItemLabelStyle(),
      {NavigationViewItemLabelStyleClass})) {
  this->SetLogicalChildren({mIcon, mText});
}

NavigationViewItem::~NavigationViewItem() = default;

void NavigationViewItem::Invoke() {
  mWasActivated = true;
  this->SetIsChecked(true);
}

Widget::EventHandlerResult NavigationViewItem::OnClick(const MouseEvent&) {
  this->Invoke();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
