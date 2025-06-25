// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "font_detail.hpp"

namespace FredEmmott::GUI::direct2d_detail {

constexpr float DIPsToPixels(const auto in) {
  return font_detail::PixelsFromDPI<96>(in);
}

constexpr float PixelsToDIPs(const auto in) {
  return font_detail::PixelsToDPI<96>(in);
}

}// namespace FredEmmott::GUI::direct2d_detail