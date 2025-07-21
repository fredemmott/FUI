// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::Button {

const ImmutableStyle& DefaultButtonStyle();
const ImmutableStyle& AccentButtonStyle();

constexpr LiteralStyleClass AccentButtonStyleClass {"Button/Accent"};

}// namespace FredEmmott::GUI::StaticTheme::Button