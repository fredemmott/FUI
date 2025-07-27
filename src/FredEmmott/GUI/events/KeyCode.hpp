// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cinttypes>

namespace FredEmmott::GUI {

// Mostly from the ASCII table
enum class KeyCode : uint8_t {
  Key_Tab = 0x09,
  Key_Return = 0x0D,
  Key_Space = 0x20,
};

}