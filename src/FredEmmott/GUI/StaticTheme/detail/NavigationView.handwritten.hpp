// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::NavigationView {

/* Absent here, but present in XAML:
 *
 * - PaneToggleButtonStyle
 * - NavigationViewPaneSearchButtonStyle
 * - NavigationViewPaneSearchButtonStyle
 * - NavigationViewOverflowButtonStyleWhenPaneOnTop
 * - NavigationViewOverflowButtonNoLabelStyleWhenPaneOnTop
 * - NavigationViewItem presenter - lots of these
 * - NavigationViewItemHeaderTextStyle
 * - NavigationViewTitleHeaderContentControlTextStyle
 */

constexpr LiteralStyleClass NavigationViewPaneExpandedStyleClass {
  "NavigationView/Pane/Expanded"};

const ImmutableStyle& NavigationViewStyle();

const ImmutableStyle& NavigationViewPaneStyle();
const ImmutableStyle& NavigationViewItemsRootStyle();

const ImmutableStyle& NavigationViewItemStyle();

const ImmutableStyle& NavigationViewContentOuterStyle();
const ImmutableStyle& NavigationViewContentHeaderStyle();
const ImmutableStyle& NavigationViewContentInnerStyle();

}// namespace FredEmmott::GUI::StaticTheme::NavigationView
