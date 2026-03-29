// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <d3d11.h>

#include <FredEmmott/GUI/Point.hpp>
#include <FredEmmott/GUI/Size.hpp>
#include <cinttypes>

namespace FredEmmott::GUI::win32_detail {

void CopySoftwareBitmap(
  ID3D11Device* const device,
  ID3D11DeviceContext* const context,
  IDXGISurface* const dest,
  const BasicPoint<uint32_t>& destOffset,
  const void* inputData,
  const BasicSize<uint32_t>& inputSize,
  uint32_t inputStride);

}