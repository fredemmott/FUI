// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "CopySoftwareBitmap.hpp"

#include <wil/com.h>

#include <FredEmmott/GUI/detail/win32_detail.hpp>

namespace FredEmmott::GUI::win32_detail {
void CopySoftwareBitmap(
  ID3D11Device* const device,
  ID3D11DeviceContext* const context,
  IDXGISurface* const dest,
  const BasicPoint<uint32_t>& destOffset,
  const void* inputData,
  const BasicSize<uint32_t>& inputSize,
  const uint32_t inputStride) {
  const D3D11_TEXTURE2D_DESC desc {
    .Width = static_cast<UINT>(inputSize.mWidth),
    .Height = static_cast<UINT>(inputSize.mHeight),
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {1, 0},
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0,
  };
  const D3D11_SUBRESOURCE_DATA initData {
    .pSysMem = inputData,
    .SysMemPitch = inputStride,
  };

  const auto destTexture = wil::com_query<ID3D11Texture2D>(dest);
  wil::com_ptr<ID3D11Texture2D> sourceTexture;
  CheckHResult(device->CreateTexture2D(&desc, &initData, sourceTexture.put()));
  context->CopySubresourceRegion(
    destTexture.get(),
    0,
    destOffset.mX,
    destOffset.mY,
    0,
    sourceTexture.get(),
    0,
    nullptr);
}

}// namespace FredEmmott::GUI::win32_detail
