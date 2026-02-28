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

}// namespace FredEmmott::GUI::StaticTheme::TitleBar