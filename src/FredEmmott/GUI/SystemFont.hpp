// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <skia/core/SkFont.h>

#include "Win32-Ganesh-D3D12.hpp"

namespace FredEmmott::GUI::SystemFont {

// https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography

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

extern const SkFont Caption;
extern const SkFont Body;
extern const SkFont BodyStrong;
extern const SkFont BodyLarge;
extern const SkFont Subtitle;
extern const SkFont Title;
extern const SkFont TitleLarge;
extern const SkFont Display;

}// namespace FredEmmott::GUI::SystemFont