// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

namespace FredEmmott::GUI::StaticTheme {
enum class Theme {
  Light,
  Dark,
  HighContrast,
};

Theme GetCurrent();

}// namespace FredEmmott::GUI::StaticTheme