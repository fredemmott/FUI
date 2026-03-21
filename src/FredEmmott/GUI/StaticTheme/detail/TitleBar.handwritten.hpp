// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/StaticTheme/TitleBar.hpp>

namespace FredEmmott::GUI::StaticTheme::TitleBar {

const ImmutableStyle& DefaultTitleBarStyle();
const ImmutableStyle& WindowMinimizeMaximizeButtonStyle();
const ImmutableStyle& WindowCloseButtonStyle();

const ImmutableStyle& TitleBarContentContainerStyle();

const ImmutableStyle& TitleBarTitleStyle();
const ImmutableStyle& TitleBarSubtitleStyle();
const ImmutableStyle& TitleBarIconStyle();

inline constexpr LiteralStyleClass TitleBarInactiveWindowStyleClass {
  "TitleBar/InactiveWindow"};
inline constexpr LiteralStyleClass TitleBarLeftButtonStyleClass {
  "TitleBar/LeftButton"};

}// namespace FredEmmott::GUI::StaticTheme::TitleBar