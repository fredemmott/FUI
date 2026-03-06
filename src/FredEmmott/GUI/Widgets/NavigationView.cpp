// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "NavigationView.hpp"

#include "FredEmmott/GUI/StaticTheme/NavigationView.hpp"
#include "Label.hpp"

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

NavigationView::NavigationView(const id_type id)
  : Widget(id, NavigationViewStyleClass, NavigationViewStyle()),
    mPane(
      new Widget(0, NavigationViewPaneStyleClass, NavigationViewPaneStyle())),
    mPaneHeader(new Widget(0, NavigationViewPaneHeaderStyleClass, {})),
    mItemsRoot(new Widget(0, NavigationViewItemsRootStyleClass, {})),
    mFooterItemsRoot(
      new Widget(0, NavigationViewFooterItemsRootStyleClass, {})),
    mContentOuter(new Widget(
      0,
      NavigationViewContentOuterStyleClass,
      NavigationViewContentOuterStyle())),
    mContentHeader(new Label(
      0,
      NavigationViewContentHeaderStyle(),
      {NavigationViewContentHeaderStyleClass})),
    mContentInner(new Widget(
      0,
      NavigationViewContentInnerStyleClass,
      NavigationViewContentInnerStyle())) {
  mPane->SetStructuralChildren({mPaneHeader, mItemsRoot, mFooterItemsRoot});
  mContentOuter->SetStructuralChildren({mContentHeader, mContentInner});
  this->SetStructuralChildren({mPane, mContentOuter});
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

}// namespace FredEmmott::GUI::Widgets