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

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style& style)
    const override;

 private:
  ImportedTexture::HandleKind mTextureHandleKind {};
  HANDLE mTexture {};
  HANDLE mFence {};
  uint64_t mFenceValue {};
  Rect mSourceRect {};
  std::optional<Rect> mDestRect {};
};

}// namespace FredEmmott::GUI::Widgets