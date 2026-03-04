// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <vector>

namespace FredEmmott::GUI {
struct Bitmap {
  // Add `stride` (bytes per pixel) property or method if formats with
  // padded rows are added in the future; e.g. if rows are required to be
  // 32-bit-aligned but pixels are not, stride will sometimes include additional
  // padding
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