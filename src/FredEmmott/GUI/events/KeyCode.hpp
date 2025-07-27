// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/utility/bitflag_enums.hpp>
#include <FredEmmott/utility/type_tag.hpp>
#include <cinttypes>

namespace FredEmmott::GUI {

// Mostly from the ASCII table
enum class KeyCode : uint8_t {
  Key_Tab = 0x09,
  Key_Return = 0x0D,
  Key_Space = 0x20,
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