// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GPUTexture.hpp"

#include "FredEmmott/GUI/Direct2DRenderer.hpp"
#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Widgets {

GPUTexture::GPUTexture(const std::size_t id)
  : Widget(id, LiteralStyleClass {"GPUTexture"}, {}) {}

GPUTexture::~GPUTexture() = default;

void GPUTexture::SetContent(
  const ImportedTexture::HandleKind kind,
  HANDLE const texture,
  HANDLE const fence,
  const uint64_t fenceValue,
  const Rect& sourceRect,
  const std::optional<Rect>& destRect) {
  mTextureHandleKind = kind;
  mTexture = texture;
  mFence = fence;
  mFenceValue = fenceValue;
  mSourceRect = sourceRect;
  mDestRect = destRect;
}

void GPUTexture::PaintOwnContent(
  Renderer* renderer,
  const Rect& widgetRect,
  const Style&) const {
  if (!mTexture) {
    return;
  }
  FUI_ASSERT(mFence);
  FUI_ASSERT(mFenceValue);
  const auto rendererImpl = direct2d_renderer_cast(renderer);
  if (!rendererImpl) {
    return;
  }
  const auto texture
    = rendererImpl->ImportTexture(mTextureHandleKind, mTexture);
  const auto fence = rendererImpl->ImportFence(mFence);

  auto dest = widgetRect;
  if (mDestRect) {
    dest.mTopLeft += mDestRect->mTopLeft;
    dest.mSize = mDestRect->mSize;
  }

  renderer->PushClipRect(widgetRect);
  rendererImpl->DrawTexture(
    mSourceRect, dest, texture.get(), fence.get(), mFenceValue);
  renderer->PopClipRect();
}

}// namespace FredEmmott::GUI::Widgets