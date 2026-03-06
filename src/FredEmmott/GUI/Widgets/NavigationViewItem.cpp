// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationViewItem.hpp"

#include "FredEmmott/GUI/StaticTheme/NavigationView.hpp"
#include "NavigationView.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace StaticTheme::NavigationView;
constexpr LiteralStyleClass NavigationViewItemIconHolderStyleClass(
  "NavigationView/Item/IconHolder");
}// namespace

NavigationViewItem::NavigationViewItem(const id_type id)
  : Widget(id, NavigationViewItemStyleClass, NavigationViewItemStyle()),
    ISelectionItem(this),
    mIconHolder(new Widget(
      0,
      NavigationViewItemIconHolderStyleClass,
      NavigationViewItemIconHolderStyle())),
    mIcon(new Label(0, NavigationViewItemIconStyle())),
    mText(new Label(
      0,
      NavigationViewItemLabelStyle(),
      {NavigationViewItemLabelStyleClass})) {
  mIconHolder->SetStructuralChildren({mIcon});
  this->SetLogicalChildren({mIconHolder, mText});
}

NavigationViewItem::~NavigationViewItem() = default;

void NavigationViewItem::Select() {
  mWasSelected = true;
  this->SetIsChecked(true);
  for (const auto item: this->GetSelectionContainer()->GetSelectionItems()) {
    if (item != this) {
      CastSelectionSibling<NavigationViewItem>(item)->SetIsChecked(false);
    }
  }
}

ISelectionContainer* NavigationViewItem::GetSelectionContainer()
  const noexcept {
  return this
    ->GetStructuralParent()// Items or FooterItems
    ->GetStructuralParent()// Pane
    ->GetStructuralParent<NavigationView>();
}

Widget::EventHandlerResult NavigationViewItem::OnClick(const MouseEvent&) {
  this->Select();
  return EventHandlerResult::StopPropagation;
}

}// namespace FredEmmott::GUI::Widgets
