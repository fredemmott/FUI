// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::MenuFlyout {

inline constexpr float MenuFlyoutIconSize = 16;
const ImmutableStyle& MenuFlyoutItemStyle();
const ImmutableStyle& MenuFlyoutStyle();
const ImmutableStyle& MenuFlyoutSeparatorStyle();

}// namespace FredEmmott::GUI::StaticTheme::MenuFlyout