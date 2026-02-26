// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "SwapChainPanel.hpp"

#include <felly/guarded_data.hpp>
#include <felly/numeric_cast.hpp>

#include "FredEmmott/GUI/Window.hpp"
#include "FredEmmott/GUI/detail/win32_detail.hpp"

#ifdef FUI_ENABLE_DIRECT2D
#include <d3d11.h>

#include "FredEmmott/GUI/Direct2DRenderer.hpp"
#endif

namespace FredEmmott::GUI::Widgets {

namespace {
using namespace win32_detail;
constexpr auto SwapChainLength = 3;
}// namespace

struct SwapChainPanel::Resources {
  struct GuardedData {
    std::optional<Submission> mContent;
    Window* mOwnerWindow {};
  };

  Resources() = delete;
  virtual ~Resources() = default;
  explicit Resources(GuardedData gd) : mGuarded(std::move(gd)) {}

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
};

SwapChainPanel::SwapChainPanel(const std::size_t id)
  : Widget(id, LiteralStyleClass {"SwapChainPanel"}, {}) {
  mResources.reset(new Resources(
    Resources::GuardedData {.mOwnerWindow = this->GetOwnerWindow()}));
}

SwapChainPanel::~SwapChainPanel() {
  mResources->mGuarded.lock()->mOwnerWindow = nullptr;
}

SwapChainPanel::SwapChain SwapChainPanel::GetSwapChain() const noexcept {
  return SwapChain {mResources};
}

SwapChainPanel::SwapChain::~SwapChain() {}
std::optional<SwapChainPanel::SwapChain::BeginFrameInfo>
SwapChainPanel::SwapChain::BeginFrame() {
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

void SwapChainPanel::SwapChain::EndFrame(
  const BeginFrameInfo& begin,
  const EndFrameInfo& end) {
  FUI_ASSERT(mStrong);

  FUI_ASSERT(begin.mSequenceNumber == mStrong->mNextFrameNumber - 1);
  const auto idx = begin.mSequenceNumber % SwapChainLength;
  mStrong->mHandlesAreNew[idx] = false;

  Submission ret {
    .mTexture = mStrong->mImportedTextures[idx].get(),
    .mFenceIsNew = end.mFenceIsNew,
    .mFenceHandle = end.mFence,
    .mFenceValue = end.mFenceValue,
  };

  auto guarded = mStrong->mGuarded.lock();
  if (guarded->mOwnerWindow) {
    guarded->mContent = std::move(ret);
    guarded->mOwnerWindow->InterruptWaitFrame();
  }
  mStrong.reset();
}

void SwapChainPanel::PaintOwnContent(
  Renderer* renderer,
  const Rect& rect,
  const Style&) const {
  const_cast<SwapChainPanel*>(this)->Init(renderer, rect.mSize);

  const auto lock = mResources->mGuarded.lock();
  if (!lock->mContent) {
    return;
  }
  auto& content = *lock->mContent;

  const auto r = mResources.get();
  if (content.mFenceIsNew || content.mFenceHandle != r->mFenceHandle) {
    r->mFenceHandle = content.mFenceHandle;
    r->mFence = renderer->ImportFence(r->mFenceHandle);
  }
  renderer->DrawTexture(
    rect, rect, content.mTexture, r->mFence.get(), content.mFenceValue);
}

void SwapChainPanel::Init(Renderer* renderer, const Size& size) {
  if (mResources && mResources->mReady.test()) {
    return;
  }
  const auto initialized = InitD3D11(renderer, size);
  FUI_ALWAYS_ASSERT(initialized);
  FUI_ALWAYS_ASSERT(!mResources->mReady.test_and_set());
  mResources->mReady.notify_all();
}

#ifdef FUI_ENABLE_DIRECT2D
bool SwapChainPanel::InitD3D11(Renderer* raw, const Size& size) {
  const auto renderer = direct2d_renderer_cast(raw);
  if (!renderer) {
    return false;
  }
  auto d3d = renderer->GetD3DDevice();
  const auto r = mResources.get();

  r->mTextureSize = {
    felly::numeric_cast<DWORD>(size.mWidth),
    felly::numeric_cast<DWORD>(size.mHeight),
  };
  const D3D11_TEXTURE2D_DESC1 desc {
    .Width = r->mTextureSize.mWidth,
    .Height = r->mTextureSize.mHeight,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc = {1, 0},
    .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
    .MiscFlags
    = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED,
  };
  for (auto i = 0; i < SwapChainLength; ++i) {
    auto& texture = r->mD3D11.mTextures[i];
    CheckHResult(d3d->CreateTexture2D1(&desc, nullptr, texture.put()));
    const auto resource = texture.query<IDXGIResource1>();
    CheckHResult(resource->CreateSharedHandle(
      nullptr,
      DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
      nullptr,
      &r->mTextureHandles[i]));
    r->mImportedTextures[i] = renderer->ImportTexture(
      ImportedTexture::HandleKind::NTHandle, r->mTextureHandles[i]);
  }

  return true;
}
#endif

}// namespace FredEmmott::GUI::Widgets