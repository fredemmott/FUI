// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <vector>

namespace FredEmmott::GUI {
/** An image in system memory (RAM), which can be read/manipulated by the CPU.
 *
 * This is in opposition to a 'texture', which lives in VRAM.
 *
 * FUI supports:
 *
 * - rendering textures
 * - one-shot importing a SoftwareBitmap as a texture
 *
 * FUI does not currently support:
 *
 * - converting a texture to a SoftwareBitmap
 * - updating textures that have been imported from a SoftwareBitmap
 *
 * Instead, you must create a new SoftwareBitmap, then re-import that.
 *
 * This functionality primarily exists to allow rendering static images in FUI.
 */
struct SoftwareBitmap {
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