// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Style.hpp>
#include <FredEmmott/GUI/SystemFont.hpp>

#include "FredEmmott/GUI/detail/immediate/CaptionResultMixin.hpp"
#include "ID.hpp"
#include "Result.hpp"

namespace FredEmmott::GUI::Immediate {

using FontIconSize = SystemFont::Usage;

using FontIconResult
  = Result<nullptr, void, immediate_detail::CaptionResultMixin>;

FontIconResult FontIcon(
  std::string_view glyph,
  FontIconSize size = FontIconSize::Body,
  ID id = ID {std::source_location::current()});

struct FontIconStackedGlyph {
  std::string_view mGlyph;
  Style mStyle;
};

FontIconResult FontIcon(
  std::initializer_list<FontIconStackedGlyph> glyph,
  FontIconSize size = FontIconSize::Body,
  ID id = ID {std::source_location::current()});

}// namespace FredEmmott::GUI::Immediate