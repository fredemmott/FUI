// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <vector>

namespace FredEmmott::GUI {
struct Bitmap {
  enum class PixelLayout {
    BGRA32,
  };
  enum class AlphaFormat {
    Premultiplied,
  };

  std::vector<std::byte> mData;

  PixelLayout mPixelLayout {};
  AlphaFormat mAlphaFormat {};

  uint16_t mWidth {};
  uint16_t mHeight {};
};
}// namespace FredEmmott::GUI