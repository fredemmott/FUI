// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>

#include "ID.hpp"

namespace FredEmmott::GUI::Immediate {

using FontIconSize = SystemFont::Usage;

void FontIcon(
  std::string_view glyph,
  FontIconSize size = FontIconSize::Body,
  ID id = ID {std::source_location::current()});

struct FontIconStackedGlyph {
  std::string_view mGlyph;
  Style mStyle;
};

void FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyph,
  FontIconSize size = FontIconSize::Body,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate