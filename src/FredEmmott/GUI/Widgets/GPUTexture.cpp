// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GPUTexture.hpp"

#include "FredEmmott/GUI/detail/immediate_detail.hpp"

namespace FredEmmott::GUI::Widgets {

GPUTexture::GPUTexture(const std::size_t id)
  : Widget(id, LiteralStyleClass {"GPUTexture"}, {}) {}

GPUTexture::~GPUTexture() {
  this->ClearImportedResourceCache();
};

void GPUTexture::SetContent(
  const ImportedTexture::HandleKind kind,
  HANDLE const texture,
  HANDLE const fence,
  const uint64_t fenceValue,
  const Rect& sourceRect,
  const std::optional<Rect>& destRect) {
  mTextureHandleKind = kind;
  mTextureHandle = texture;
  mFenceHandle = fence;
  mFenceValue = fenceValue;
  mSourceRect = sourceRect;
  mDestRect = destRect;
}

void GPUTexture::ClearImportedResourceCache() {
  const auto textures = std::exchange(mTextures, {});
  const auto fences = std::exchange(mFences, {});
  for (auto&& it: textures | std::views::values) {
    it.mFlag->Wait();
  }
  for (auto&& it: fences | std::views::values) {
    it.mFlag->Wait();
  }
}

void GPUTexture::PaintOwnContent(
  Renderer* renderer,
  const Rect& widgetRect,
  const Style&) const {
  if (!mTextureHandle) {
    return;
  }
  FUI_ASSERT(mFenceHandle);
  FUI_ASSERT(mFenceValue);

  auto& texture = mTextures[mTextureHandle];
  if (!texture.mTexture) {
    texture.mTexture
      = renderer->ImportTexture(mTextureHandleKind, mTextureHandle);
  }
  auto& fence = mFences[mFenceHandle];
  if (!fence.mFence) {
    fence.mFence = renderer->ImportFence(mFenceHandle);
  }
  texture.mFlag = fence.mFlag = renderer->GetGPUCompletionFlagForCurrentFrame();

  auto dest = widgetRect;
  if (mDestRect) {
    dest.mTopLeft += mDestRect->mTopLeft;
    dest.mSize = mDestRect->mSize;
  }

  renderer->PushClipRect(widgetRect);
  renderer->DrawTexture(
    mSourceRect, dest, texture.mTexture.get(), fence.mFence.get(), mFenceValue);
  renderer->PopClipRect();
}

}// namespace FredEmmott::GUI::Widgets