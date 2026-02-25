// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <felly/guarded_data.hpp>

#include "Widget.hpp"

namespace FredEmmott::GUI::Widgets {

struct SwapChainBeginFrameInfo {
  uint64_t mSequenceNumber {};
  BasicSize<uint16_t> mTextureSize {};

  bool mMustFlushCachedHandles {};

  HANDLE mTexture {};
};

struct SwapChainEndFrameInfo {
  HANDLE mFence {};
  uint64_t mFenceValue {};
  bool mFenceIsNew {true};
};

class SwapChainPanel final : public Widget {
 public:
  explicit SwapChainPanel(std::size_t id);
  ~SwapChainPanel() override;

  [[nodiscard]]
  SwapChainBeginFrameInfo BeginFrame();
  void EndFrame(const SwapChainBeginFrameInfo&, const SwapChainEndFrameInfo&);

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  struct Resources;
  struct Submission {
    ImportedTexture* mTexture {};
    bool mFenceIsNew {};
    HANDLE mFenceHandle {};
    uint64_t mFenceValue {};
  };

  std::atomic_flag mReady;
  std::unique_ptr<Resources> mResources;
  std::atomic<uint64_t> mNextFrameNumber {};

  felly::guarded_data<std::optional<Submission>> mContent;

  void Init(Renderer* renderer, const Size& size);
#ifdef FUI_ENABLE_DIRECT2D
  [[nodiscard]]
  bool InitD3D11(Renderer*, const Size&);
#endif
};

}// namespace FredEmmott::GUI::Widgets