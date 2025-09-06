// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/utility/bitflag_enums.hpp>
#include <FredEmmott/utility/type_tag.hpp>
#include <cinttypes>

namespace FredEmmott::GUI {

// This is a mirror of the Windows VK_ enum for portability, given that it
// already exists, and already handles anything we care about
enum class KeyCode : uint8_t {
  Key_Tab = 0x09,
  Key_Return = 0x0D,
  Key_Backspace = 0x08,
  Key_Escape = 0x1B,
  Key_Space = 0x20,
  Key_End = 0x23,
  Key_Home = 0x24,
  Key_LeftArrow = 0x25,
  Key_UpArrow = 0x26,
  Key_RightArrow = 0x27,
  Key_DownArrow = 0x28,
  Key_Delete = 0x2E,
  Key_A = 0x41,
  Key_B = 0x42,
  Key_C = 0x43,
  Key_D = 0x44,
  Key_E = 0x45,
  Key_F = 0x46,
  Key_G = 0x47,
  Key_H = 0x48,
  Key_I = 0x49,
  Key_J = 0x4A,
  Key_K = 0x4B,
  Key_L = 0x4C,
  Key_M = 0x4D,
  Key_N = 0x4E,
  Key_O = 0x4F,
  Key_P = 0x50,
  Key_Q = 0x51,
  Key_R = 0x52,
  Key_S = 0x53,
  Key_T = 0x54,
  Key_U = 0x55,
  Key_V = 0x56,
  Key_W = 0x57,
  Key_X = 0x58,
  Key_Y = 0x59,
  Key_Z = 0x5A,
};

enum class KeyModifier : uint8_t {
  Modifier_None = 0,
  Modifier_Shift = 1 << 0,
  Modifier_Control = 1 << 1,
  Modifier_Alt = 1 << 2,
};
constexpr bool is_bitflag_enum(utility::type_tag_t<KeyModifier>) {
  return true;
}
static_assert(utility::bitflag_enum<KeyModifier>);

}// namespace FredEmmott::GUI