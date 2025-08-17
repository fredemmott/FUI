// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/utility/bitflag_enums.hpp>
#include <FredEmmott/utility/type_tag.hpp>
#include <cinttypes>

namespace FredEmmott::GUI {

enum class KeyCode : uint8_t {
  // Matching ASCII
  Key_Tab = 0x09,
  Key_Return = 0x0D,
  Key_Escape = 0x1B,
  Key_Space = 0x20,

  // Matching Win32 VKEY_*
  Key_End = 0x23,
  Key_Home = 0x24,
  Key_LeftArrow = 0x25,
  Key_UpArrow = 0x26,
  Key_RightArrow = 0x27,
  Key_DownArrow = 0x28,
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