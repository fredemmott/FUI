// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/utility/bitflag_enums.hpp>
#include <cinttypes>

namespace FredEmmott::GUI {

using namespace utility::bitflag_enums;

enum class MouseButton : uint8_t {
  None = 0,
  Left = 1,
  Middle = 2,
  Right = 4,
  X1 = 8,
  X2 = 16,
};
consteval bool is_bitflag_enum(std::type_identity<MouseButton>) {
  return true;
}
static_assert(std::to_underlying(MouseButton::Left | MouseButton::Right) == 5);
using MouseButtons = MouseButton;

}// namespace FredEmmott::GUI