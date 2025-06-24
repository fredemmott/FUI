// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetHexColorValue.hpp"

#include <format>
#include <stdexcept>

std::string GetHexColorValue(std::string_view hex) {
  if (!hex.starts_with('#')) [[unlikely]] {
    throw std::runtime_error {
      std::format("Color value `{}` does not start with `#`", hex)};
  }
  switch (hex.size()) {
    case 7:
      return std::format(
        "Color::Constant::FromARGB32(0xFF, 0x{}, 0x{}, 0x{})",
        hex.substr(1, 2),
        hex.substr(3, 2),
        hex.substr(5, 2));
    case 9:
      return std::format(
        "Color::Constant::FromARGB32(0x{}, 0x{}, 0x{}, 0x{})",
        hex.substr(1, 2),
        hex.substr(3, 2),
        hex.substr(5, 2),
        hex.substr(7, 2));
    default:
      throw std::runtime_error {
        std::format("Color value `{}` is not a valid hex color", hex)};
  }
}
