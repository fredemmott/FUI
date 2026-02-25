// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/Renderer.hpp>
#include <optional>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

class GPUTexture final : public Widget {
 public:
  explicit GPUTexture(std::size_t id);
  ~GPUTexture() override;

  void SetContent(
    ImportedTexture::HandleKind,
    HANDLE texture,
    HANDLE fence,
    uint64_t fenceValue,
    const Rect& sourceRect,
    const std::optional<Rect>& destRect = std::nullopt);

  void ClearImportedResourceCache();

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style& style)
    const override;

 private:
  ImportedTexture::HandleKind mTextureHandleKind {};
  HANDLE mTextureHandle {};
  HANDLE mFenceHandle {};
  uint64_t mFenceValue {};
  Rect mSourceRect {};
  std::optional<Rect> mDestRect {};

  struct TextureAndFlag {
    std::unique_ptr<ImportedTexture> mTexture;
    std::shared_ptr<GPUCompletionFlag> mFlag;
  };
  struct FenceAndFlag {
    std::unique_ptr<ImportedFence> mFence;
    std::shared_ptr<GPUCompletionFlag> mFlag;
  };

  mutable std::unordered_map<HANDLE, TextureAndFlag> mTextures {};
  mutable std::unordered_map<HANDLE, FenceAndFlag> mFences {};
};

}// namespace FredEmmott::GUI::Widgets