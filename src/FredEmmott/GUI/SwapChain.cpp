// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SwapChain.hpp"

#include "SwapChain_Resources.hpp"
#include "Widgets/SwapChainPanel.hpp"

namespace FredEmmott::GUI {

SwapChain::~SwapChain() = default;

std::optional<SwapChain::BeginFrameInfo> SwapChain::BeginFrame() {
  FUI_ASSERT(!mStrong);
  mStrong = mWeak.lock();
  if (!mStrong) {
    return {};
  }

  mStrong->mReady.wait(false);
  const auto frameNumber = mStrong->mNextFrameNumber++;
  const auto idx = frameNumber % SwapChainLength;
  return BeginFrameInfo {
    .mSequenceNumber = frameNumber,
    .mTextureSize = {
      static_cast<uint16_t>(mStrong->mTextureSize.mWidth),
      static_cast<uint16_t>(mStrong->mTextureSize.mHeight),
      },
    .mMustFlushCachedHandles = mStrong->mHandlesAreNew[idx],
    .mTexture = mStrong->mTextureHandles[idx],
  };
}

void SwapChain::EndFrame(const BeginFrameInfo& begin, const EndFrameInfo& end) {
  FUI_ASSERT(mStrong);

  FUI_ASSERT(begin.mSequenceNumber == mStrong->mNextFrameNumber - 1);
  const auto idx = begin.mSequenceNumber % SwapChainLength;
  mStrong->mHandlesAreNew[idx] = false;

  Widgets::SwapChainPanel::Submission ret {
    .mTexture = mStrong->mImportedTextures[idx].get(),
    .mFenceIsNew = end.mFenceIsNew,
    .mFenceHandle = end.mFence,
    .mFenceValue = end.mFenceValue,
  };

  if (auto guarded = mStrong->mGuarded.lock(); guarded->mOwnerWindow) {
    guarded->mContent = std::move(ret);
    guarded->mOwnerWindow->InterruptWaitFrame();
  }
  mStrong.reset();
}

SwapChain::Resources::~Resources() {
  if (mCompletionFlag) {
    mCompletionFlag->Wait();
  }
}

}// namespace FredEmmott::GUI