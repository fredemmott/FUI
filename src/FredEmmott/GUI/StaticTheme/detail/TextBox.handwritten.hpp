// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>

namespace FredEmmott::GUI::StaticTheme::TextBox {

const ImmutableStyle& DefaultTextBoxStyle();
const ImmutableStyle& DefaultTextBoxButtonStyle();

// e.g. clear buttons are not visible when the text box is not focused
inline constexpr LiteralStyleClass
  TextBoxButtonInvisibleWhenInactiveStyleClass {
    "TextBox/Buttons/InvisibleWhenInactive"};

}// namespace FredEmmott::GUI::StaticTheme::TextBox
