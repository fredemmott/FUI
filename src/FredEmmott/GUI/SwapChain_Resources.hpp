// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/config.hpp>
#include <array>
#include <atomic>
#include <felly/guarded_data.hpp>

#include "SwapChain.hpp"
#include "Widgets/SwapChainPanel.hpp"

#ifdef FUI_ENABLE_DIRECT2D
#include "Direct2DRenderer.hpp"
#endif
#ifdef FUI_ENABLE_SKIA
#include "SkiaRenderer.hpp"
#endif

namespace FredEmmott::GUI {
struct SwapChain::Resources final {
  using SwapChainPanel = Widgets::SwapChainPanel;

  struct GuardedData {
    std::optional<SwapChainPanel::Submission> mContent;
    Window* mOwnerWindow {};
  };

  Resources() = delete;
  ~Resources();
  explicit Resources(GuardedData gd) : mGuarded(std::move(gd)) {}

  std::shared_ptr<GPUCompletionFlag> mCompletionFlag;
  felly::guarded_data<GuardedData> mGuarded;

  std::array<HANDLE, SwapChainLength> mTextureHandles {};
  std::array<std::unique_ptr<ImportedTexture>, SwapChainLength>
    mImportedTextures {};
  std::array<bool, SwapChainLength> mHandlesAreNew {true};
  BasicSize<DWORD> mTextureSize;

  HANDLE mFenceHandle {};
  std::unique_ptr<ImportedFence> mFence;

  std::atomic_flag mReady;
  std::atomic<uint64_t> mNextFrameNumber {};
#ifdef FUI_ENABLE_DIRECT2D
  struct D3D11 {
    std::array<wil::com_ptr<ID3D11Texture2D1>, SwapChainLength> mTextures;
  };
  D3D11 mD3D11 {};
#endif
#ifdef FUI_ENABLE_SKIA
  struct D3D12 {
    std::array<wil::com_ptr<ID3D12Resource>, SwapChainLength> mTextures;
  };
  D3D12 mD3D12 {};
#endif
};

}// namespace FredEmmott::GUI