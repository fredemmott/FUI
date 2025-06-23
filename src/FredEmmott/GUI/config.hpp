// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#define FUI_ENABLE_SKIA

namespace FredEmmott::GUI {
#ifndef NDEBUG
constexpr bool Debug = true;
#else
constexpr bool Debug = false;
#endif
}// namespace FredEmmott::GUI