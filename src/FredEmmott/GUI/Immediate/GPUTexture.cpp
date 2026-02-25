// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GPUTexture.hpp"

#include "FredEmmott/GUI/Widgets/GPUTexture.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Immediate {

GPUTextureResult GPUTexture(
  const ImportedTexture::HandleKind kind,
  HANDLE const texture,
  HANDLE const fence,
  const uint64_t fenceValue,
  const Rect& sourceRect,
  const std::optional<Rect>& destRect,
  const ID id) {
  const auto w = immediate_detail::ChildlessWidget<Widgets::GPUTexture>(id);
  w->SetContent(kind, texture, fence, fenceValue, sourceRect, destRect);
  return {w};
}
}// namespace FredEmmott::GUI::Immediate