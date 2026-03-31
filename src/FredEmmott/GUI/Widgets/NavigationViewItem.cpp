// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationViewItem.hpp"

#include "FredEmmott/GUI/StaticTheme/NavigationView.hpp"
#include "NavigationView.hpp"

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace StaticTheme::NavigationView;
constexpr LiteralStyleClass NavigationViewItemIconHolderStyleClass {
  "NavigationView/Item/IconHolder"};
constexpr LiteralStyleClass NavigationViewItemIconStyleClass {
  "NavigationView/Item/Icon"};
using PillState = detail::SelectionPill::State;
}// namespace

NavigationViewItem::NavigationViewItem(Window* const window)
  : Widget(
      window,
      NavigationViewItemStyleClass,
      NavigationViewItemStyle(),
      {PseudoClasses::ExplicitMouseButtonSink}),
    ISelectionItem(this),
    mIconHolder(new Widget(
      window,
      NavigationViewItemIconHolderStyleClass,
      NavigationViewItemIconHolderStyle())),
    mIcon(new Label(
      window,
      NavigationViewItemIconStyleClass,
      NavigationViewItemIconStyle())),
    mText(new Label(
      window,
      NavigationViewItemLabelStyleClass,
      NavigationViewItemLabelStyle())) {
  mIconHolder->SetStructuralChildren({mIcon});
  this->SetStructuralChildren({mIconHolder, mText});
}

NavigationViewItem::~NavigationViewItem() = default;

void NavigationViewItem::Select() {
  if (this->IsSelected()) {
    return;
  }

  const auto peers = this->GetSelectionContainer()->GetSelectionItems();
  const auto selectedPeer = std::ranges::find_if(
    peers, [this](auto peer) { return (peer != this) && peer->IsSelected(); });

  this->MarkActivated();
  mWasSelected = true;
  this->SetIsChecked(true);

  if (selectedPeer == peers.end()) {
    mSelectionPill.Transition(PillState::Selected);
    return;
  }

  for (const auto item:
       std::ranges::subrange(selectedPeer, std::ranges::end(peers))) {
    if (item != this) {
      CastSelectionSibling<NavigationViewItem>(item)->SetIsChecked(false);
    }
  }

  const auto selfIt = std::ranges::find(peers, this);
  FUI_ASSERT(selfIt != peers.end());
  if (selfIt < selectedPeer) {
    mSelectionPill.Transition(PillState::GainingSelectionFromBelow);
    CastSelectionSibling<NavigationViewItem>(*selectedPeer)
      ->mSelectionPill.Transition(PillState::LosingSelectionToAbove);
  } else {
    mSelectionPill.Transition(PillState::GainingSelectionFromAbove);
    CastSelectionSibling<NavigationViewItem>(*selectedPeer)
      ->mSelectionPill.Transition(PillState::LosingSelectionToBelow);
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

Widget::EventHandlerResult NavigationViewItem::OnClick(const MouseEvent& e) {
  if (
    e.Get<MouseEvent::ButtonReleaseEvent>().mReleasedButtons
    == MouseButton::Left) {
    this->Select();
  }
  std::ignore = Widget::OnClick(e);
  return EventHandlerResult::StopPropagation;
}

FrameRateRequirement NavigationViewItem::GetFrameRateRequirement()
  const noexcept {
  return Widget::GetFrameRateRequirement()
    + mSelectionPill.GetFrameRateRequirement();
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
