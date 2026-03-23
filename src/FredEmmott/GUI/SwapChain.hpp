// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <memory>

#include "Renderer.hpp"
#include "Size.hpp"

namespace FredEmmott::GUI {

namespace Widgets {
class SwapChainPanel;
}

class SwapChain final {
 private:
  static constexpr auto SwapChainLength = 3;
  friend class Widgets::SwapChainPanel;
  struct Resources;

  std::weak_ptr<Resources> mWeak;
  std::shared_ptr<Resources> mStrong;

 public:
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

  constexpr SwapChain() = default;
  explicit SwapChain(std::weak_ptr<Resources> weak) : mWeak(std::move(weak)) {}
  SwapChain(const SwapChain&) = default;
  SwapChain(SwapChain&&) = default;
  SwapChain& operator=(const SwapChain&) = default;
  SwapChain& operator=(SwapChain&&) = default;
  ~SwapChain();

  [[nodiscard]]
  std::optional<BeginFrameInfo> BeginFrame();
  void EndFrame(const BeginFrameInfo&, const EndFrameInfo&);

  bool operator==(const SwapChain& other) const noexcept {
    return mWeak.lock() == other.mWeak.lock();
  }
};

}// namespace FredEmmott::GUI