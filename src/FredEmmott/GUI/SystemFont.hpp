// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkFont.h>

#include "Win32-Ganesh-D3D12.hpp"

namespace FredEmmott::GUI {
class Font;
}

namespace FredEmmott::GUI::SystemFont {

// https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography

enum class Usage {
  Caption,
  Body,
  BodyStrong,
  BodyLarge,
  Subtitle,
  Title,
  TitleLarge,
  Display,
};

Font Resolve(Usage) noexcept;

Font Caption() noexcept;
Font Body() noexcept;
Font BodyStrong() noexcept;
Font BodyLarge() noexcept;
Font Subtitle() noexcept;
Font Title() noexcept;
Font TitleLarge() noexcept;
Font Display() noexcept;

}// namespace FredEmmott::GUI::SystemFont