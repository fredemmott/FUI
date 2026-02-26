// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Widget.hpp"

struct ID3D11Device3;
struct ID3D12Device;

namespace FredEmmott::GUI::Widgets {

class SwapChainPanel final : public Widget {
 private:
  struct Resources;

 public:
  struct SwapChain final {
    constexpr SwapChain() = default;
    explicit SwapChain(std::weak_ptr<Resources> weak)
      : mWeak(std::move(weak)) {}
    SwapChain(const SwapChain&) = default;
    SwapChain(SwapChain&&) = default;
    SwapChain& operator=(const SwapChain&) = default;
    SwapChain& operator=(SwapChain&&) = default;
    ~SwapChain();

    struct BeginFrameInfo {
      uint64_t mSequenceNumber {};
      BasicSize<uint16_t> mTextureSize {};

      bool mMustFlushCachedHandles {};

      HANDLE mTexture {};
    };

    struct EndFrameInfo {
      HANDLE mFence {};
      uint64_t mFenceValue {};
      bool mFenceIsNew {true};
    };

    [[nodiscard]]
    std::optional<BeginFrameInfo> BeginFrame();
    void EndFrame(const BeginFrameInfo&, const EndFrameInfo&);

   private:
    std::weak_ptr<Resources> mWeak;
    std::shared_ptr<Resources> mStrong;
  };

  explicit SwapChainPanel(std::size_t id);
  ~SwapChainPanel() override;

  SwapChain GetSwapChain() const noexcept;

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  struct Submission {
    ImportedTexture* mTexture {};
    bool mFenceIsNew {};
    HANDLE mFenceHandle {};
    uint64_t mFenceValue {};
  };

  std::shared_ptr<Resources> mResources;

  void Init(Renderer* renderer, const Size& size);

#ifdef FUI_ENABLE_DIRECT2D
  [[nodiscard]]
  bool InitD3D11(ID3D11Device3*, Renderer*, const Size&);
  [[nodiscard]]
  bool InitD3D11(Renderer*, const Size&);
#endif
#ifdef FUI_ENABLE_SKIA
  [[nodiscard]]
  bool InitD3D12(ID3D12Device*, Renderer*, const Size&);
  [[nodiscard]]
  bool InitSkia(Renderer*, const Size&);
#endif
};

}// namespace FredEmmott::GUI::Widgets