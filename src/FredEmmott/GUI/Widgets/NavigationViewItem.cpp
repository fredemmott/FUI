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
using PillState = detail::SelectionPill::State;
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

  const auto peers = this->GetSelectionContainer()->GetSelectionItems();
  const auto it = std::ranges::find_if(
    peers, [this](auto peer) { return (peer != this) && peer->IsSelected(); });
  if (it == peers.end()) {
    mSelectionPill.Transition(PillState::Selected);
    return;
  }

  for (const auto item: std::ranges::subrange(it, std::ranges::end(peers))) {
    if (item != this) {
      CastSelectionSibling<NavigationViewItem>(item)->SetIsChecked(false);
    }
  }

  const auto selfIt = std::ranges::find(peers, this);
  FUI_ASSERT(selfIt != peers.end());
  if (selfIt < it) {
    mSelectionPill.Transition(PillState::GainingSelectionFromBelow);
    CastSelectionSibling<NavigationViewItem>(*it)->mSelectionPill.Transition(
      PillState::LosingSelectionToAbove);
  } else {
    mSelectionPill.Transition(PillState::GainingSelectionFromAbove);
    CastSelectionSibling<NavigationViewItem>(*it)->mSelectionPill.Transition(
      PillState::LosingSelectionToBelow);
  }
}

ISelectionContainer* NavigationViewItem::GetSelectionContainer()
  const noexcept {
  return this
    ->GetStructuralParent()// Items or FooterItems
    ->GetStructuralParent()// Pane
    ->GetStructuralParent<NavigationView>();
}

void NavigationViewItem::Tick(
  const std::chrono::steady_clock::time_point& now) {
  mSelectionPill.Tick(now);
  Widget::Tick(now);
}

Widget::EventHandlerResult NavigationViewItem::OnClick(const MouseEvent&) {
  this->Select();
  return EventHandlerResult::StopPropagation;
}

FrameRateRequirement NavigationViewItem::GetFrameRateRequirement()
  const noexcept {
  if (mSelectionPill.IsAnimating()) {
    return FrameRateRequirement::SmoothAnimation {};
  }
  return Widget::GetFrameRateRequirement();
}

void NavigationViewItem::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style&) const {
  const Rect pillRect {
    Point {
      rect.GetLeft(),
      rect.GetTop(),
    },
    Size {
      NavigationViewSelectionIndicatorWidth,
      rect.GetHeight(),
    },
  };
  mSelectionPill.Paint(
    renderer,
    pillRect,
    NavigationViewSelectionIndicatorForeground,
    NavigationViewSelectionIndicatorHeight);
}

}// namespace FredEmmott::GUI::Widgets
