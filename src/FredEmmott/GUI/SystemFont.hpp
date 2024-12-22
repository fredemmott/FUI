// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkFont.h>

#include "Win32-Ganesh-D3D12.hpp"

namespace FredEmmott::GUI {

// https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography

struct SystemFont {
  enum class Weight {
    Light = 300,
    SemiLight = 350,
    Regular = 400,
    SemiBold = 600,
    Bold = 700,
  };

  enum class Height {
    Caption = 16,
    Body = 20,
    BodyStrong = 20,
    BodyLarge = 24,
    Subtitle = 28,
    Title = 36,
    TitleLarge = 52,
    Display = 92,
  };

  static const SkFont Caption;
  static const SkFont Body;
  static const SkFont BodyStrong;
  static const SkFont BodyLarge;
  static const SkFont Subtitle;
  static const SkFont Title;
  static const SkFont TitleLarge;
  static const SkFont Display;
};
}// namespace FredEmmott::GUI