// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationView.hpp"

#include "FredEmmott/GUI/StaticTheme/NavigationView.hpp"
#include "FredEmmott/GUI/Windows/Win32Window.hpp"
#include "Label.hpp"
#include "NavigationViewBackButton.hpp"
#include "NavigationViewTogglePaneButton.hpp"
#include "TitleBar.hpp"

namespace FredEmmott::GUI::Widgets {
namespace {
using namespace StaticTheme::NavigationView;
constexpr LiteralStyleClass NavigationViewStyleClass {"NavigationView"};
constexpr LiteralStyleClass NavigationViewPaneStyleClass {
  "NavigationView/Pane"};
constexpr LiteralStyleClass NavigationViewPaneHeaderStyleClass {
  "NavigationView/Pane/Header"};
constexpr LiteralStyleClass NavigationViewItemsRootStyleClass {
  "NavigationView/Pane/Items"};
constexpr LiteralStyleClass NavigationViewFooterItemsRootStyleClass {
  "NavigationView/Pane/FooterItems"};
constexpr LiteralStyleClass NavigationViewContentHeaderStyleClass {
  "NavigationView/ContentHeader"};
constexpr LiteralStyleClass NavigationViewContentOuterStyleClass {
  "NavigationView/ContentOuter"};
constexpr LiteralStyleClass NavigationViewContentInnerStyleClass {
  "NavigationView/ContentInner"};
}// namespace

NavigationView::NavigationView(Window* const window)
  : Widget(window, NavigationViewStyleClass, NavigationViewStyle()),
    ISelectionContainer(this),
    mPane(new Widget(
      window,
      NavigationViewPaneStyleClass,
      NavigationViewPaneStyle())),
    mPaneHeader(new Widget(
      window,
      NavigationViewPaneHeaderStyleClass,
      NavigationViewPaneHeaderStyle())),
    mItemsRoot(new Widget(
      window,
      NavigationViewItemsRootStyleClass,
      NavigationViewItemsRootStyle())),
    mFooterItemsRoot(new Widget(
      window,
      NavigationViewFooterItemsRootStyleClass,
      NavigationViewFooterItemsRootStyle())),
    mContentOuter(new Widget(
      window,
      NavigationViewContentOuterStyleClass,
      NavigationViewContentOuterStyle())),
    mContentHeader(new Label(
      window,
      NavigationViewContentHeaderStyleClass,
      NavigationViewContentHeaderStyle())),
    mContentInner(new Widget(
      window,
      NavigationViewContentInnerStyleClass,
      NavigationViewContentInnerStyle())) {
  mBackButton = new NavigationViewBackButton(window);
  mTogglePaneButton = new NavigationViewTogglePaneButton(window, this);
  mPaneHeader->SetStructuralChildren({mTogglePaneButton});

  mPane->SetStructuralChildren(
    {mBackButton, mPaneHeader, mItemsRoot, mFooterItemsRoot});
  mContentOuter->SetStructuralChildren({mContentHeader, mContentInner});
  this->SetStructuralChildren({mPane, mContentOuter});
  mPane->AddStyleClass(NavigationViewPaneExpandedStyleClass);
}

NavigationView::~NavigationView() = default;

void NavigationView::SetHeaderText(const std::string_view text) {
  if (text.empty()) {
    mContentHeader->SetMutableStyles(Style().Display(YGDisplayNone));
    return;
  }
  mContentHeader->SetMutableStyles(Style().Display(YGDisplayFlex));
  mContentHeader->SetText(text);
}

std::vector<ISelectionItem*> NavigationView::GetSelectionItems()
  const noexcept {
  // TODO (C++23): use std::views::concat when it's available in MSVC
  return std::array {
           std::views::all(mItemsRoot->GetStructuralChildren()),
           std::views::all(mFooterItemsRoot->GetStructuralChildren()),
         }
  | std::views::join | std::views::transform([](Widget* p) {
           return dynamic_cast<ISelectionItem*>(p);
         })
    | std::views::filter([](ISelectionItem* const p) { return p != nullptr; })
    | std::ranges::to<std::vector>();
}
void NavigationView::TogglePaneIsExpanded() {
  mPaneIsExpanded = !mPaneIsExpanded;
  mPane->ToggleStyleClass(
    NavigationViewPaneExpandedStyleClass, mPaneIsExpanded);
}
void NavigationView::IntegrateWithTitleBar() {
  if (std::exchange(mIsTitleBarAttached, true)) {
    return;
  }

  const auto window = static_cast<Win32Window*>(this->GetOwnerWindow());
  const auto titleBar = window->GetTitleBar();
  FUI_ASSERT(titleBar);

  const auto backDisabled = mBackButton->IsDisabled();

  auto paneChildren = mPane->GetStructuralChildren();
  std::erase(paneChildren, std::exchange(mBackButton, nullptr));
  mPane->SetStructuralChildren(paneChildren);

  auto headerChildren = mPaneHeader->GetStructuralChildren();
  std::erase(headerChildren, std::exchange(mTogglePaneButton, nullptr));
  mPaneHeader->SetStructuralChildren(headerChildren);

  mBackButton = new NavigationViewBackButton(window);
  mBackButton->SetIsDirectlyDisabled(backDisabled);
  mTogglePaneButton = new NavigationViewTogglePaneButton(window, this);

  titleBar->SetLeftWidgets({mBackButton, mTogglePaneButton});
  mPaneHeader->SetMutableStyles(Style().Display(YGDisplayNone));
}

}// namespace FredEmmott::GUI::Widgets